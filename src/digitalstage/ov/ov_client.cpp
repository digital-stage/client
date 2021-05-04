//
// Created by Tobias Hegemann on 01.05.21.
//

#include "ov_client.h"

using namespace DigitalStage::Api;
using namespace DigitalStage::Types;

bool isValidOvStage(const Stage& stage)
{
  return stage.ovIpv4 && stage.ovPort && stage.ovPin;
}

ov_client::ov_client(ov_render_tascar_t* renderer_,
                     DigitalStage::Api::Client* client_)
    : renderer(renderer_), client(client_), insideOvStage(false)
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::ov_client(...)" << std::endl;
#endif
  // Add handler
  client->ready.connect(&ov_client::onReady, this);
  client->stageJoined.connect(&ov_client::onStageJoined, this);
  client->stageLeft.connect(&ov_client::onStageLeft, this);
  client->soundCardAdded.connect(&ov_client::onSoundCardAdded, this);
  client->soundCardChanged.connect(&ov_client::onSoundCardChanged, this);
  client->remoteAudioTrackAdded.connect(&ov_client::onRemoteAudioTrackAdded,
                                        this);
  client->remoteAudioTrackRemoved.connect(&ov_client::onRemoteAudioTrackRemoved,
                                          this);
}
ov_client::~ov_client()
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::~ov_client" << std::endl;
#endif
}

void ov_client::onReady(const Store* store)
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
void ov_client::onStageJoined(const DigitalStage::Types::ID_TYPE& stageId,
                              const DigitalStage::Types::ID_TYPE&,
                              const DigitalStage::Api::Store* store)
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

void ov_client::handleStageJoined(const Stage& stage,
                                  const DigitalStage::Api::Store* store)
{
#ifdef SHOWDEBUG
  std::cout << "ov_client::handleStageJoined" << std::endl;
#endif
  insideOvStage = isValidOvStage(stage);
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::handleStageJoined - insideOvStage" << std::endl;
#endif
    // We have to build up session
    // First the sound card
    auto localDevice = store->getLocalDevice();
    if(!localDevice) {
      std::cerr
          << "Internal error: try to join a stage, but have no local device yet"
          << std::endl;
    }
    if( localDevice->soundCardId ) {
      auto soundCard = store->getSoundCard(*localDevice->soundCardId);
      if(soundCard) {
        setSoundCard(*soundCard, store);
        renderer->start_audiobackend();
      } else {
        std::cerr
            << "Internal error: Could not find sound card " << *localDevice->soundCardId
            << std::endl;
      }
    }
    // And sync the stage members (and their tracks, includes implicit this
    // stage member)
    for(auto& stageMember : store->getStageMembersByStage(stage._id)) {
      syncStageMember(stageMember._id, store);
    }
    renderer->start_session();
  }
}

void ov_client::onStageLeft(const DigitalStage::Api::Store*)
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

void ov_client::onSoundCardAdded(
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

void ov_client::setSoundCard(const DigitalStage::Types::SoundCard& soundCard,
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

  // Sync input channels
  syncInputChannels(soundCard, store);
}

void ov_client::onSoundCardChanged(const DigitalStage::Types::ID_TYPE& id,
                                   const json& update,
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

void ov_client::onRemoteAudioTrackAdded(
    const DigitalStage::Types::remote_audio_track_t& track,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onRemoteAudioTrackAdded" << std::endl;
#endif
    syncStageMember(track.stageMemberId, store);
  }
}

void ov_client::onRemoteAudioTrackRemoved(
    const DigitalStage::Types::remote_audio_track_t& track,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
#ifdef SHOWDEBUG
    std::cout << "ov_client::onRemoteAudioTrackRemoved" << std::endl;
#endif
    syncStageMember(track.stageMemberId, store);
  }
}

void ov_client::syncInputChannels(SoundCard soundCard, const Store* store)
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
  auto localAudioTracks = store->getLocalAudioTracks();
  if(soundCard.inputChannels.empty()) {
    // Remove all local audio tracks related to this device
    for(auto& localAudioTrack : localAudioTracks) {
      if(localAudioTrack.deviceId == localDeviceId) {
        client->send(SendEvents::REMOVE_LOCAL_AUDIO_TRACK, localAudioTrack._id);
      }
    }
  } else {
    // Propagate local tracks for all new input channels
    for(auto& channel : soundCard.inputChannels) {
      if(channel.second) {
        // Lookup local audio track
        if(std::none_of(
               localAudioTracks.begin(), localAudioTracks.end(),
               [channel, localDeviceId](const local_audio_track_t& track) {
                 return track.deviceId == localDeviceId &&
                        track.ovSourcePort == channel.first;
               })) {
          // Create local audio track
          nlohmann::json payload = {{"type", "ov"},
                                    {"ovSourcePort", channel.first}};
          client->send(SendEvents::CREATE_LOCAL_AUDIO_TRACK, payload);
        }
      }
    }
    // Clean up deprecated tracks
    for(auto& localAudioTrack : localAudioTracks) {
      if(localAudioTrack.ovSourcePort &&
         localAudioTrack.deviceId == localDeviceId) {
        std::cout << "Checking " << localAudioTrack._id << std::endl;
        if(soundCard.inputChannels.count(*localAudioTrack.ovSourcePort) == 0 ||
           !soundCard.inputChannels.at(*localAudioTrack.ovSourcePort)) {
          std::cout << "Channel is gone " << localAudioTrack._id << std::endl;
          client->send(SendEvents::REMOVE_LOCAL_AUDIO_TRACK,
                       localAudioTrack._id);
        }
      }
    }
  }
}

void ov_client::syncStageMember(const DigitalStage::Types::ID_TYPE& id,
                                const DigitalStage::Api::Store* store)
{
  auto stageMember = store->getStageMember(id);
  if(!stageMember) {
    std::cerr << "Internal error: Could not find stage member " << id
              << std::endl;
    return;
  }
  auto localDevice = store->getLocalDevice();
  if(!localDevice) {
    std::cerr << "Internal error: No local device specified, but inside "
                 "an ov stage"
              << std::endl;
    return;
  }
  auto user = store->getUser(stageMember->userId);
  if(!user) {
    std::cerr << "Internal error: Could not find user " << stageMember->userId
              << std::endl;
    return;
  }
#ifdef SHOWDEBUG
  std::cout << "ov_client::syncStageMember(" << user->name << ")" << std::endl;
#endif
  auto stageDevices = store->getStageDevicesByStageMember(id);
  for(auto& stageDevice : stageDevices) {
    auto tracks = store->getRemoteAudioTracksByStageDevice(stageDevice._id);
    stage_device_t stage_device;
    stage_device.id = stageDevice.order;
    stage_device.label = user->name;
    stage_device.senderjitter =
        *localDevice
             ->ovSenderJitter; // TODO: use StageDevice instead of local device?
    stage_device.receiverjitter =
        *localDevice->ovReceiverJitter; // TODO: use StageDevice instead of
                                        // local device?
    if(*store->getStageMemberId() == id) {
      // This stage member (former user) may use different devices, so we have
      // to separate the remote audio tracks via device ID
      std::vector<device_channel_t> deviceChannels;
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
          deviceChannels.push_back(device_channel);
        }
      }
    }
    if(stageDevice.deviceId == localDevice->_id) {
      renderer->set_thisdev(stage_device);
    } else {
      renderer->add_stage_device(stage_device);
    }
  }
}