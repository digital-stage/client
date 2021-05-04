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
    : renderer(renderer_), client(client_)
{
  // Add handler
  client->ready.connect(&ov_client::onReady, this);
  client->soundCardAdded.connect(&ov_client::handleSoundCardAdded, this);
}
ov_client::~ov_client() {}

void ov_client::onReady(const Store* store)
{
  // Check if inside an ov stage
  auto stageId = store->getStageId();
  if(stageId) {
    auto stage = store->getStage(*stageId);
    if(stage) {
      insideOvStage = isValidOvStage(*stage);
      if(insideOvStage) {
        // We have to build up session
      }
    } else {
      std::cerr << "Internal error: could not find stage " << *stageId
                << std::endl;
    }
  }
}

void ov_client::handleSoundCardAdded(
    const DigitalStage::Types::soundcard_t soundCard,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
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
}
void ov_client::handleSoundCardChanged(const DigitalStage::Types::ID_TYPE& id,
                                       const json& update,
                                       const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
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

void ov_client::handleRemoteAudioTrackAdded(
    const DigitalStage::Types::remote_audio_track_t& track,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
    syncStageMember(track.stageMemberId, store);
  }
}

void ov_client::handleRemoteAudioTrackRemoved(
    const DigitalStage::Types::remote_audio_track_t& track,
    const DigitalStage::Api::Store* store)
{
  if(insideOvStage) {
    syncStageMember(track.stageMemberId, store);
  }
}

void ov_client::syncInputChannels(soundcard_t soundCard, const Store* store)
{
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
  auto tracks = store->getRemoteAudioTracksByStageMember(id);
  stage_device_t stage_device;
  // TODO: Problem: each device has an ID, not each stage member ...
  stage_device.id = stageMember->order;
  stage_device.label = user->name;
  stage_device.senderjitter = *localDevice->ovSenderJitter;
  stage_device.receiverjitter = *localDevice->ovReceiverJitter;
  if(*store->getStageMemberId() == id) {
    // This stage member (former user) may use different devices, so we have to
    // separate the remote audio tracks via device ID
    std::vector<device_channel_t> localDeviceTracks;
    std::vector<device_channel_t> remoteDeviceTracks;
    for(auto& track : tracks) {
      if(track.type == "ov") {
        device_channel_t device_channel;
        device_channel.id = track._id;
        device_channel.gain = track.volume;
        device_channel.directivity = track.directivity;
        device_channel.position = {track.x, track.y, track.z};
        if(track.deviceId == localDevice->_id) {
          // This is a remote representation of an local audio track
          device_channel.sourceport = *track.ovSourcePort;
          localDeviceTracks.push_back(device_channel);
        } else {
          // This is a track of another ov device
          remoteDeviceTracks.push_back(device_channel);
        }
      }
    }
  }
  for(auto& track : tracks) {
    if(track.deviceId == *localDeviceId) {
      // This is a remote representation of an local audio track
      stage_device_t stage_device;
      stage_device.id = track._id;
      stage_device.label = track.volume;
      stage_device.gain = track.volume;
      stage_device.channels = track.volume;
      renderer->this_dev();
    } else {
      // This may be the this or another stage member, but at least it's a
      // different device
    }
  }
}