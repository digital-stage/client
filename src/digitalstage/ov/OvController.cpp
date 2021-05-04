//
// Created by Tobias Hegemann on 28.04.21.
//

#include "OvController.h"

using namespace nlohmann;
using namespace DigitalStage::Api;
using namespace DigitalStage::Types;

OvController::OvController(ov_render_tascar_t* renderer_,
                           DigitalStage::Api::Client* client_)
    : isRunning(false), shouldStart(false), client(client_), renderer(renderer_)
{
  init();
}
OvController::~OvController()
{
  std::cout << "OvController::~OvController()" << std::endl;
  // stop();
  isRunning = false;
}

bool isChannelRepresented(
    const std::vector<local_audio_track_t>& localAudioTracks,
    const std::string& channel)
{
  for(auto& localAudioTrack : localAudioTracks)
    if(localAudioTrack.ovSourcePort &&
       localAudioTrack.ovSourcePort == channel) {
      return true;
    }
  return false;
}

void OvController::init()
{
  client->ready.connect([&](const Store*) {
    std::cout << "OvController::ready TRIGGER" << std::endl;
    if(shouldStart && hasSoundCard()) {
      // We should start
      startService();
    }
  });
  client->deviceChanged.connect(
      [&](const ID_TYPE& id, const nlohmann::json& update, const Store* store) {
        if(id == store->getLocalDeviceId()) {
          if(update.count("soundCardId")) {
            if(shouldStart && !isRunning) {
              // Now start, since we've been waiting for the sound card
              startService();
            } else {
              // Just update sound card
              updateSoundCard(getCurrentSoundCard());
            }
          }
        }
      });
  client->soundCardChanged.connect(
      [&](const ID_TYPE& id, const nlohmann::json& update, const Store* store) {
        std::cout << "OvController::soundCardChanged" << std::endl;
        if(store->isReady()) {
          auto localDevice = store->getLocalDevice();
          if(localDevice && localDevice->soundCardId &&
             *localDevice->soundCardId == id) {
            std::cout << "OvController::soundCardChanged CORRECT SOUNDCARD"
                      << std::endl;
            // Current sound card has changed
            auto soundCard = getCurrentSoundCard();
            updateSoundCard(soundCard);
            if(update.count("inputChannels") > 0) {
              std::cout << "Checking local audio tracks" << std::endl;
              // Propagate local tracks for all new input channels
              auto localTracks = store->getLocalAudioTracks();
              for(auto& channel : soundCard.inputChannels) {
                if(channel.second) {
                  // Lookup local audio track
                  if(!isChannelRepresented(localTracks, channel.first)) {
                    // Create local audio track
                    nlohmann::json payload = {{"type", "ov"},
                                              {"ovSourcePort", channel.first}};
                    client->send(SendEvents::CREATE_LOCAL_AUDIO_TRACK, payload);
                  }
                }
              }
              // Clean up deprecated tracks
              for(auto& track : localTracks) {
                if(track.ovSourcePort) {
                  std::cout << "Checking " << track._id << std::endl;
                  if(soundCard.inputChannels.count(*track.ovSourcePort) == 0 ||
                     !soundCard.inputChannels.at(*track.ovSourcePort)) {
                    std::cout << "Channel is gone " << track._id << std::endl;
                    client->send(SendEvents::REMOVE_LOCAL_AUDIO_TRACK,
                                 track._id);
                  }
                }
              }
            }
          }
        }
      });
  client->remoteAudioTrackAdded.connect([&](const remote_audio_track_t track, const Store* store) {
    auto stageMemberId = store->getStageMemberId();
  });
  client->remoteAudioTrackRemoved.connect([&](const ID_TYPE& id, const Store* store) {

  });
  client->stageJoined.connect(
      [&](const ID_TYPE&, const ID_TYPE&, const Store* store) {
        std::cout << "OvController::stageJoined TRIGGER" << std::endl;
        if(isRunning) {
          // First stop current service
          stopService();
        }
        if(shouldStart && store->isReady() && hasSoundCard()) {
          // Everything is fine, we can start
          startService();
        }
      });
  client->stageLeft.connect([&](const Store*) {
    std::cout << "OvController::stageLeft TRIGGER" << std::endl;
    if(isRunning) {
      stopService();
    }
  });
}

void OvController::start()
{
  std::cout << "OvController::start()" << std::endl;
  shouldStart = true;
  if(!isRunning && client->getStore()->isReady()) {
    startService();
  }
}

void OvController::stop()
{
  std::cout << "OvController::stop()" << std::endl;
  shouldStart = false;
  if(isRunning) {
    stopService();
  }
}

void OvController::startService()
{
  std::cout << "OvController::startService()" << std::endl;
  auto store = client->getStore();
  auto stageId = store->getStageId();
  if(stageId) {
    auto stage = store->getStage(*stageId);
    if(!stage) {
      std::cerr << "Internal error: Could not find stage " << *stageId
                << std::endl;
      return;
    }
    if(stage->audioType == "ov") {
      // Inside an ov stage
      // Now start everything
      isRunning = true;
      std::cout << "Set sound card" << std::endl;
      updateSoundCard(getCurrentSoundCard());
      std::cout << "Starting service" << std::endl;
      renderer->start_audiobackend();
      std::cout << "Service started" << std::endl;
    } else {
      std::cout << "Stage is unsupported" << std::endl;
    }
  }
}
void OvController::stopService()
{
  std::cout << "OvController::stopService()" << std::endl;
  renderer->stop_audiobackend();
  isRunning = false;
}

void OvController::updateSoundCard(soundcard_t soundCard)
{
  std::cout << "OvController::updateSoundCard" << std::endl;
  audio_device_t audioDevice;
  audioDevice.srate = soundCard.sampleRate;
  audioDevice.periodsize = soundCard.periodSize;
  audioDevice.numperiods = soundCard.numPeriods;
  audioDevice.drivername = soundCard.driver ? *soundCard.driver : "jack";
  audioDevice.devicename = soundCard.uuid;
  renderer->configure_audio_backend(audioDevice);
  std::cout << "OvController::updateSoundCard FINISHED" << std::endl;
}

void OvController::updateLocalStageMember()
{
  // Get local stage member
  auto localDevice = getLocalDevice();
  auto currentSoundCard = getCurrentSoundCard();
  auto localStageMemberPair = getLocalStageMember();
  auto localStageMember = localStageMemberPair.first;
  auto localUser = localStageMemberPair.second;
  // Transform stage device
  stage_device_t stage_device;
  stage_device.id = localStageMember.order;
  stage_device.label = localUser.name;
  stage_device.receiverjitter = *localDevice.ovReceiverJitter;
  stage_device.senderjitter = *localDevice.ovSenderJitter;
  // Get "local" remote audio tracks
  auto audioTracks = client->getStore()->getRemoteAudioTracksByStageMember(
      localStageMember._id);
  if(!audioTracks.empty()) {
    ReducedStageMember reduced_stage_member =
        reduceStageMember(localStageMember);
    for(auto& audioTrack : audioTracks) {
      device_channel_t channel;
      channel.id = audioTrack._id;
      channel.sourceport =
          audioTrack.ovSourcePort ? *audioTrack.ovSourcePort : "";
      // SIMPLE VERSION
      channel.position = reduced_stage_member.position;
      channel.directivity = reduced_stage_member.directivity;
      channel.gain =
          reduced_stage_member.muted ? 0 : reduced_stage_member.volume;
      stage_device.channels.push_back(channel);
    }
  }
  renderer->set_thisdev(stage_device);
}

Device OvController::getLocalDevice() noexcept(false)
{
  auto localDevice = client->getStore()->getLocalDevice();
  if(!localDevice) {
    throw std::runtime_error("Internal error: Local device is not ready");
  }
  return *localDevice;
}

DigitalStage::Types::soundcard_t
OvController::getSoundCard(const ID_TYPE& id) noexcept(false)
{
  std::cout << "getSoundCard: 1" << std::endl;
  auto soundCard = client->getStore()->getSoundCard(id);
  std::cout << "getSoundCard: 2" << std::endl;
  if(!soundCard) {
    throw std::runtime_error("Internal error: Could not find sound card " + id);
  }
  return *soundCard;
}

DigitalStage::Types::soundcard_t
OvController::getCurrentSoundCard() noexcept(false)
{
  std::cout << "getCurrentSoundCard: 1" << std::endl;
  auto localDevice = getLocalDevice();
  std::cout << "getCurrentSoundCard: 2" << std::endl;
  if(!localDevice.soundCardId) {
    throw std::runtime_error("Internal error: No sound card assigned");
  }
  std::cout << "getCurrentSoundCard: 3" << std::endl;
  return getSoundCard(*localDevice.soundCardId);
}

std::pair<DigitalStage::Types::StageMember, DigitalStage::Types::user_t>
OvController::getLocalStageMember() noexcept(false)
{
  auto store = client->getStore();
  auto localStageMemberId = store->getStageMemberId();
  if(!localStageMemberId) {
    throw std::runtime_error("Internal error: Local stage member is not ready");
  }
  return getStageMember(*localStageMemberId);
}

std::pair<DigitalStage::Types::StageMember, DigitalStage::Types::user_t>
OvController::getStageMember(const ID_TYPE& id) noexcept(false)
{
  auto store = client->getStore();
  auto stageMember = store->getStageMember(id);
  if(!stageMember) {
    throw std::runtime_error("Internal error: Could not found stage member " +
                             id);
  }
  auto user = store->getUser(stageMember->userId);
  if(!user) {
    throw std::runtime_error("Internal error: Could not find user " +
                             stageMember->userId +
                             " assigned to stage member " + id);
  }
  return std::make_pair(*stageMember, *user);
}

bool OvController::hasSoundCard()
{
  auto localDevice = client->getStore()->getLocalDevice();
  if(!localDevice) {
    return false;
  }
  return localDevice->soundCardId.has_value();
}

ReducedStageMember
OvController::reduceStageMember(const StageMember& stageMember) noexcept(false)
{
  std::cout << "OvController::reduceStageMember()" << std::endl;
  auto store = client->getStore();
  auto localDeviceId = store->getLocalDeviceId();
  if(!localDeviceId) {
    throw std::runtime_error("Internal error: No local device available");
  }
  auto group = store->getGroup(stageMember.groupId);
  auto customStageMemberVolume =
      store->getCustomStageMemberVolumeByStageMemberAndDevice(stageMember._id,
                                                              *localDeviceId);
  auto customStageMemberPosition =
      store->getCustomStageMemberPositionByStageMemberAndDevice(stageMember._id,
                                                                *localDeviceId);
  auto customGroupVolume = store->getCustomGroupVolumeByGroupAndDevice(
      stageMember.groupId, *localDeviceId);
  auto customGroupPosition = store->getCustomGroupPositionByGroupAndDevice(
      stageMember.groupId, *localDeviceId);

  // Calculate stage member volume
  ReducedStageMember reduced;

  reduced.volume = customStageMemberVolume ? customStageMemberVolume->volume
                                           : stageMember.volume;
  reduced.volume = customGroupVolume
                       ? (customGroupVolume->volume * reduced.volume)
                       : (group->volume * reduced.volume);

  reduced.muted = stageMember.muted;

  if(customStageMemberVolume) {
    reduced.muted = customStageMemberVolume->muted;
  }
  if(customStageMemberPosition) {
    reduced.position = {
        customStageMemberPosition->x,
        customStageMemberPosition->y,
        customStageMemberPosition->z,
    };
    reduced.orientation = {
        customStageMemberPosition->rZ,
        customStageMemberPosition->rY,
        customStageMemberPosition->rX,
    };
    reduced.directivity = customStageMemberPosition->directivity;
  } else {
    reduced.position = {
        stageMember.x,
        stageMember.y,
        stageMember.z,
    };
    reduced.orientation = {
        stageMember.rZ,
        stageMember.rY,
        stageMember.rX,
    };
    reduced.directivity = stageMember.directivity;
  }
  if(customGroupVolume) {
    reduced.muted = customGroupVolume->muted || reduced.muted;
  }
  if(customGroupPosition) {
    reduced.position = {reduced.position.x * customGroupPosition->x,
                        reduced.position.y * customGroupPosition->y,
                        reduced.position.z * customGroupPosition->z};
    reduced.orientation = {
        reduced.orientation.z * customGroupPosition->rZ,
        reduced.orientation.y * customGroupPosition->rY,
        reduced.orientation.x * customGroupPosition->rX,
    };
  } else {
    reduced.muted = group->muted || reduced.muted;
    reduced.position = {reduced.position.x * group->x,
                        reduced.position.y * group->y,
                        reduced.position.z * group->z};
    reduced.orientation = {
        reduced.orientation.z * group->rZ,
        reduced.orientation.y * group->rY,
        reduced.orientation.x * group->rX,
    };
  }
  return reduced;
}