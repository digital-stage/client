//
// Created by Tobias Hegemann on 01.05.21.
//

#include "ov_ds_sockethandler_t.h"

using namespace DigitalStage::Api;
using namespace DigitalStage::Types;

bool isValidOvStage(const Stage& stage)
{
  return stage.ovIpv4 && stage.ovPort && stage.ovPin;
}

ov_ds_sockethandler_t::ov_ds_sockethandler_t(ov_render_base_t* renderer_,
                                             DigitalStage::Api::Client* client_)
    : renderer(renderer_), client(client_), insideOvStage(false)
{
}
ov_ds_sockethandler_t::~ov_ds_sockethandler_t()
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::~ov_client" << std::endl;
#endif
  unlisten();
}

void ov_ds_sockethandler_t::onReady(const Store* store)
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::onReady" << std::endl;
#endif
  // Check if inside an ov stage
  auto stageId = store->getStageId();
  if(stageId) {
    auto stage = store->getStage(*stageId);
    if(stage) {
      handleStageJoined(*stage, store);
    } else {
      std::cerr << "Internal error: could not find stage " << *stageId
                << std::endl;
    }
  }
}
void ov_ds_sockethandler_t::onStageJoined(
    const DigitalStage::Types::ID_TYPE& stageId,
    const DigitalStage::Types::ID_TYPE&, const DigitalStage::Api::Store* store)
{
  if(store->isReady()) {
    auto stage = store->getStage(stageId);
    if(stage) {
      handleStageJoined(*stage, store);
    } else {
      std::cerr << "Internal error: could not find stage " << stageId
                << std::endl;
    }
  }
}

void ov_ds_sockethandler_t::handleStageJoined(
    const Stage& stage, const DigitalStage::Api::Store* store)
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::handleStageJoined" << std::endl;
#endif
  if(isValidOvStage(stage)) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::handleStageJoined - insideOvStage" << std::endl;
#endif
    try {
      // Fetch all necessary models
      auto localStageDevice = store->getStageDevice();
      if(!localStageDevice)
        throw std::runtime_error("Local stage device missing");
      auto localDevice = store->getLocalDevice();
      if(!localDevice)
        throw std::runtime_error("Local device missing");
      if(!localDevice->soundCardId)
        throw std::runtime_error("No sound card specified");
      auto soundCard = store->getSoundCard(*localDevice->soundCardId);
      if(!soundCard)
        throw std::runtime_error("Sound card missing");

      // We have to build up session
      // - set the relay server
      renderer->set_relay_server(*stage.ovIpv4, *stage.ovPort, *stage.ovPin);
      // - configure audio
      setSoundCard(*soundCard, store);
      // - set room settings
      render_settings_t stageSettings;
      stageSettings.id = localStageDevice->order;
      stageSettings.roomsize.x = stage.width;
      stageSettings.roomsize.y = stage.length;
      stageSettings.roomsize.z = stage.height;
      stageSettings.roomsize.z = stage.height;
      stageSettings.absorption = stage.absorption;
      stageSettings.damping = stage.reflection;
      stageSettings.reverbgain =
          localDevice->ovReverbGain ? *localDevice->ovReverbGain : 0.6;
      stageSettings.renderreverb =
          localDevice->ovRenderReverb && *localDevice->ovRenderReverb;
      stageSettings.renderism =
          localDevice->ovRenderISM && *localDevice->ovRenderISM;
      std::vector<std::string> outputChannels;
      for(auto& channel : soundCard->outputChannels) {
        if(channel.second) {
          outputChannels.push_back(channel.first);
        }
      }
      stageSettings.outputport1 =
          outputChannels.empty() ? "" : outputChannels[0];
      stageSettings.outputport2 =
          outputChannels.size() > 1 ? "" : outputChannels[1];
      stageSettings.rawmode = localDevice->ovRawMode && *localDevice->ovRawMode;
      stageSettings.rectype =
          localDevice->ovReceiverType ? *localDevice->ovReceiverType : "ortf";
      stageSettings.secrec = 0.0;
      stageSettings.egogain =
          localDevice->egoGain ? *localDevice->egoGain : 0.6;
      stageSettings.mastergain = 1.0;
      stageSettings.peer2peer = localDevice->ovP2p && *localDevice->ovP2p;
      renderer->set_render_settings(stageSettings, localStageDevice->order);

      // And sync the stage members (and their tracks, includes implicit this
      // stage member)
      /*for(auto& stageMember : store->getStageMembersByStage(stage._id)) {
        syncStageMember(stageMember._id, store);
      }*/
      syncWholeStage(store);

      // if(!renderer->is_audio_active())
      //   renderer->start_audiobackend();
      renderer->restart_session_if_needed();
      insideOvStage = true;
    }
    catch(std::exception& exception) {
      std::cerr << "Internal error: " << exception.what() << std::endl;
    }
  }
}

void ov_ds_sockethandler_t::onStageChanged(
    const DigitalStage::Types::ID_TYPE& stageId, const nlohmann::json& update,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
    auto currentStageId = store->getStageId();
    if(currentStageId && stageId == currentStageId) {
      // This refers the current and active ov stage
      // Only update if some of the relay server settings changed
      if(update.count("ovIpv4") > 0 || update.count("ovPort") > 0 ||
         update.count("ovPin") > 0) {
        auto stage = store->getStage(stageId);
        if(stage && isValidOvStage(*stage)) {
          renderer->set_relay_server(*stage->ovIpv4, *stage->ovPort,
                                     *stage->ovPin);
        } else {
          std::cerr << "Internal error: could not find stage " << stageId
                    << std::endl;
        }
      }
    }
  }
}

void ov_ds_sockethandler_t::onStageLeft(const DigitalStage::Api::Store*)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onStageLeft" << std::endl;
#endif
    insideOvStage = false;
    renderer->clear_stage();
    renderer->stop_audiobackend();
  }
}

void ov_ds_sockethandler_t::onDeviceChanged(
    const std::string&, const nlohmann::json& update,
    const DigitalStage::Api::Store* store)
{
  if(update.count("soundCardId")) {
    if(!insideOvStage) {
      auto stageId = store->getStageId();
      if(stageId) {
        handleStageJoined(*store->getStage(*stageId), store);
      }
    }
  }
}

void ov_ds_sockethandler_t::onSoundCardAdded(
    const DigitalStage::Types::SoundCard& soundCard,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onSoundCardAdded" << std::endl;
#endif
    auto localDevice = store->getLocalDevice();
    if(localDevice && localDevice->soundCardId == soundCard._id) {
      setSoundCard(soundCard, store);
    }
  }
}

void ov_ds_sockethandler_t::setSoundCard(
    const DigitalStage::Types::SoundCard& soundCard,
    const DigitalStage::Api::Store* store)
{
  // Create or replace TASCAR audio configuration
  audio_device_t audioDevice;
  audioDevice.srate = soundCard.sampleRate;
  audioDevice.periodsize = soundCard.periodSize;
  audioDevice.numperiods = soundCard.numPeriods;
  audioDevice.drivername = soundCard.driver ? *soundCard.driver : "jack";
  audioDevice.devicename = soundCard.uuid;
  renderer->configure_audio_backend(audioDevice);

  bool session_was_active(renderer->is_session_active());
  if(session_was_active)
    renderer->end_session();
  renderer->stop_audiobackend();
  renderer->start_audiobackend();
  if(session_was_active)
    renderer->require_session_restart();

  // Sync input channels
  syncInputChannels(soundCard, store);
}

void ov_ds_sockethandler_t::onSoundCardChanged(
    const DigitalStage::Types::ID_TYPE& id, const json& update,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onSoundCardChanged" << std::endl;
#endif
    auto soundCard = store->getSoundCard(id);
    if(soundCard) {
      // Update TASCAR audio configuration (if necessary)
      if(update.count("sampleRate") > 0 || update.count("periodSize") > 0 ||
         update.count("numPeriods") > 0 || update.count("driver") > 0) {
        audio_device_t audioDevice;
        audioDevice.srate = soundCard->sampleRate;
        audioDevice.periodsize = soundCard->periodSize;
        audioDevice.numperiods = soundCard->numPeriods;
        audioDevice.drivername =
            soundCard->driver ? *soundCard->driver : "jack";
        audioDevice.devicename = soundCard->uuid;
        renderer->configure_audio_backend(audioDevice);
      }
      syncInputChannels(*soundCard, store);
    } else {
      std::cerr << "Internal error: Could not find sound card " << id
                << std::endl;
    }
  }
}

void ov_ds_sockethandler_t::onRemoteAudioTrackAdded(
    const DigitalStage::Types::AudioTrack&,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onRemoteAudioTrackAdded" << std::endl;
#endif
    // syncStageMember(track.stageMemberId, store);
    syncWholeStage(store);

    renderer->restart_session_if_needed();
  }
}

void ov_ds_sockethandler_t::onRemoteAudioTrackRemoved(
    const DigitalStage::Types::AudioTrack&,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onRemoteAudioTrackRemoved" << std::endl;
#endif
    // syncStageMember(track.stageMemberId, store);
    syncWholeStage(store);
  }
}

void ov_ds_sockethandler_t::syncInputChannels(SoundCard soundCard,
                                              const Store* store)
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::syncInputChannels" << std::endl;
#endif
  // Sync input channels
  auto localDeviceId = store->getLocalDeviceId();
  if(!localDeviceId) {
    std::cerr << "Internal error: No local device ID specified, but inside "
                 "an ov stage"
              << std::endl;
    return;
  }
  auto audioTracks = store->getAudioTracks();
  if(soundCard.inputChannels.empty()) {
    // Remove all local audio tracks related to this device
    for(auto& audioTrack : audioTracks) {
      if(audioTrack.deviceId == localDeviceId) {
        client->send(SendEvents::REMOVE_AUDIO_TRACK, audioTrack._id);
      }
    }
  } else {
    // Propagate local tracks for all new input channels
    for(auto& channel : soundCard.inputChannels) {
      if(channel.second) {
        // Lookup local audio track
        if(std::none_of(audioTracks.begin(), audioTracks.end(),
                        [channel, localDeviceId](const AudioTrack& track) {
                          return track.deviceId == localDeviceId &&
                                 track.ovSourcePort == channel.first;
                        })) {
          // Create local audio track
          nlohmann::json payload = {{"type", "ov"},
                                    {"ovSourcePort", channel.first}};
          client->send(SendEvents::CREATE_AUDIO_TRACK, payload);
        }
      }
    }
    // Clean up deprecated tracks
    for(auto& audioTrack : audioTracks) {
      if(audioTrack.ovSourcePort &&
          audioTrack.deviceId == localDeviceId) {
        std::cout << "Checking " << audioTrack._id << std::endl;
        if(soundCard.inputChannels.count(*audioTrack.ovSourcePort) == 0 ||
           !soundCard.inputChannels.at(*audioTrack.ovSourcePort)) {
          std::cout << "Channel is gone " << audioTrack._id << std::endl;
          client->send(SendEvents::REMOVE_AUDIO_TRACK,
                       audioTrack._id);
        }
      }
    }
  }
}

void ov_ds_sockethandler_t::syncStageMember(
    const DigitalStage::Types::ID_TYPE& id,
    const DigitalStage::Api::Store* store)
{
  try {
    auto stageMember = store->getStageMember(id);
    if(!stageMember)
      throw std::runtime_error("Could not find stage member " + id);
    auto localDevice = store->getLocalDevice();
    if(!localDevice)
      throw std::runtime_error("No local device specified");
    auto user = store->getUser(stageMember->userId);
    if(!user)
      throw std::runtime_error("Could not find user " + stageMember->userId);
#ifdef SHOWDEBUG
    std::cout << "ov_client::syncStageMember(" << user->name << ")"
              << std::endl;
#endif
    auto stageDevices = store->getStageDevicesByStageMember(id);
    for(auto& stageDevice : stageDevices) {
      stage_device_t stage_device;
      stage_device.id = stageDevice.order;
      stage_device.label = user->name;
      stage_device.senderjitter =
          *localDevice->ovSenderJitter; // TODO: use StageDevice instead of
                                        // local device?
      stage_device.receiverjitter =
          *localDevice->ovReceiverJitter; // TODO: use StageDevice instead of
      std::vector<device_channel_t> device_channels;
      auto tracks = store->getAudioTracksByStageDevice(stageDevice._id);
      for(auto& track : tracks) {
        if(track.type == "ov") {
          device_channel_t device_channel;
          device_channel.id = track._id;
          device_channel.gain = track.volume;
          device_channel.directivity = track.directivity;
          device_channel.position = {track.x, track.y, track.z};
          if(stageDevice.deviceId == localDevice->_id) {
            device_channel.sourceport = *track.ovSourcePort;
          }
          device_channels.push_back(device_channel);
        }
      }
      stage_device.channels = device_channels;

      // TODO: REPLACE WITH DATA MODEL
      stage_device.position = {0, 0, 0};
      stage_device.orientation = {0, 0, 0};
      stage_device.mute = false;

      stage_device.sendlocal = true;

      if(stageDevice.deviceId == localDevice->_id) {
        std::cout << "THIS DEV" << std::endl;
        renderer->set_thisdev(stage_device);
      } else {
        std::cout << "FOREIGN DEV" << std::endl;
        renderer->add_stage_device(stage_device);
      }
    }
  }
  catch(std::exception& exception) {
    std::cerr << "Internal error: " << exception.what() << std::endl;
  }
}

void ov_ds_sockethandler_t::syncWholeStage(
    const DigitalStage::Api::Store* store)
{
  try {
    auto stageDevices = store->getStageDevices();
    std::map<stage_device_id_t, stage_device_t> stage_devices;
    for(auto& stageDevice : stageDevices) {
      auto localDevice = store->getLocalDevice();
      if(!localDevice)
        throw std::runtime_error("No local device specified");
      auto user = store->getUser(stageDevice.userId);
      if(!user)
        throw std::runtime_error("Could not find user " + stageDevice.userId);

      stage_device_t stage_device;
      stage_device.id = stageDevice.order;
      stage_device.label = user->name;
      stage_device.senderjitter =
          *localDevice->ovSenderJitter; // TODO: use StageDevice instead of
                                        // local device?
      stage_device.receiverjitter =
          *localDevice->ovReceiverJitter; // TODO: use StageDevice instead of
      std::vector<device_channel_t> device_channels;
      auto tracks = store->getAudioTracksByStageDevice(stageDevice._id);
      for(auto& track : tracks) {
        if(track.type == "ov") {
          device_channel_t device_channel;
          device_channel.id = track._id;
          device_channel.gain = track.volume;
          device_channel.directivity = track.directivity;
          device_channel.position = {track.x, track.y, track.z};
          if(stageDevice.deviceId == localDevice->_id) {
            device_channel.sourceport = *track.ovSourcePort;
          }
          device_channels.push_back(device_channel);
        }
      }
      stage_device.channels = device_channels;

      // TODO: REPLACE WITH DATA MODEL
      stage_device.position = {0, 0, 0};
      stage_device.orientation = {0, 0, 0};
      stage_device.mute = false;

      stage_device.sendlocal = true;

      stage_devices[stage_device.id] = stage_device;
      if(stageDevice.deviceId == localDevice->_id) {
        std::cout << "THIS DEV" << std::endl;
        renderer->set_thisdev(stage_device);
      }
    }
    renderer->set_stage(stage_devices);
  }
  catch(std::exception& exception) {
    std::cerr << "Internal error: " << exception.what() << std::endl;
  }
}

void ov_ds_sockethandler_t::listen()
{
  // Add handler
  client->ready.connect(&ov_ds_sockethandler_t::onReady, this);
  client->deviceChanged.connect(&ov_ds_sockethandler_t::onDeviceChanged, this);
  client->stageJoined.connect(&ov_ds_sockethandler_t::onStageJoined, this);
  client->stageChanged.connect(&ov_ds_sockethandler_t::onStageChanged, this);
  client->stageLeft.connect(&ov_ds_sockethandler_t::onStageLeft, this);
  client->soundCardAdded.connect(&ov_ds_sockethandler_t::onSoundCardAdded,
                                 this);
  client->soundCardChanged.connect(&ov_ds_sockethandler_t::onSoundCardChanged,
                                   this);
  client->audioTrackAdded.connect(
      &ov_ds_sockethandler_t::onRemoteAudioTrackAdded, this);
  client->audioTrackRemoved.connect(
      &ov_ds_sockethandler_t::onRemoteAudioTrackRemoved, this);
}

void ov_ds_sockethandler_t::unlisten()
{
  // Remove handler
  client->ready.disconnect(&ov_ds_sockethandler_t::onReady, this);
  client->deviceChanged.disconnect(&ov_ds_sockethandler_t::onDeviceChanged,
                                   this);
  client->stageJoined.disconnect(&ov_ds_sockethandler_t::onStageJoined, this);
  client->stageChanged.disconnect(&ov_ds_sockethandler_t::onStageChanged, this);
  client->stageLeft.disconnect(&ov_ds_sockethandler_t::onStageLeft, this);
  client->soundCardAdded.disconnect(&ov_ds_sockethandler_t::onSoundCardAdded,
                                    this);
  client->soundCardChanged.disconnect(
      &ov_ds_sockethandler_t::onSoundCardChanged, this);
  client->audioTrackAdded.disconnect(
      &ov_ds_sockethandler_t::onRemoteAudioTrackAdded, this);
  client->audioTrackRemoved.disconnect(
      &ov_ds_sockethandler_t::onRemoteAudioTrackRemoved, this);

  if(renderer) {
    renderer->clear_stage();
    renderer->stop_audiobackend();
  }
}
