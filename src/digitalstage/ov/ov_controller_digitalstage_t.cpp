//
// Created by Tobias Hegemann on 28.04.21.
//

#include "ov_controller_digitalstage_t.h"

using namespace DigitalStage::Api;
using namespace DigitalStage::Types;

ov_controller_digitalstage_t::ov_controller_digitalstage_t(
    ov_render_tascar_t* renderer_, DigitalStage::Api::Client* client_)
    : isRunning(false), insideStage(false), renderer(renderer_), client(client_)
{
  init();
}
ov_controller_digitalstage_t::~ov_controller_digitalstage_t()
{
  stop();
}

void ov_controller_digitalstage_t::init()
{
  client->deviceChanged.connect(
      [&](const ID_TYPE&, const nlohmann::json& update, const Store*) {
        if(update.count("soundCardId") > 0) {
          // Change sound card
          setSoundCard(update["soundCardId"]);
        }
      });
  client->stageJoined.connect(&ov_controller_digitalstage_t::handleStageJoined,
                              this);
  client->stageLeft.connect(&ov_controller_digitalstage_t::handleStageLeft,
                            this);
  client->stageChanged.connect(
      &ov_controller_digitalstage_t::handleStageChanged, this);
}

void ov_controller_digitalstage_t::handleStageJoined(const ID_TYPE& stageId,
                                                     const ID_TYPE&,
                                                     const Store* store)
{
  std::cout << "[ov_controller_digitalstage_t] handleStageJoined()"
            << std::endl;
  auto stage = store->getStage(stageId);
  if(stage->audioType == "ov") {
    // Stage is OV
    insideStage = true;
    if(stage && stage->ovIpv4 && stage->ovPort && stage->ovPin) {
      // Set relay server
      renderer->set_relay_server(*stage->ovIpv4, *stage->ovPort, *stage->ovPin);
    }
    if(isRunning) {
      // Start
      startInternal(*stage, store);
    }
  }
}
void ov_controller_digitalstage_t::handleStageLeft(const Store*)
{
  std::cout << "[ov_controller_digitalstage_t] handleStageLeft()" << std::endl;
  if(insideStage) {
    insideStage = false;
    if(isRunning) {
      stopInternal();
    }
  }
}

void ov_controller_digitalstage_t::handleStageChanged(
    const ID_TYPE& stageId, const nlohmann::json& update, const Store* store)
{
  std::cout << "[ov_controller_digitalstage_t] handleStageChanged()"
            << std::endl;
  if(insideStage) {
    if(update.count("ovIpv4") > 0 || update.count("ovPort") > 0 ||
       update.count("ovPin") > 0) {
      auto stage = store->getStage(stageId);
      if(stage && stage->ovIpv4 && stage->ovPort && stage->ovPin) {
        // Update relay server
        renderer->set_relay_server(*stage->ovIpv4, *stage->ovPort,
                                   *stage->ovPin);
      }
    }
  }
}

void ov_controller_digitalstage_t::start()
{
  std::lock_guard<std::recursive_mutex> lock(this->mutex);
  std::cout << "[ov_controller_digitalstage_t] start() -> ";
  if(!isRunning) {
    isRunning = true;
    auto store = client->getStore();
    if(store->isReady()) {
      auto stageId = store->getStageId();
      if(stageId) {
        auto stage = store->getStage(*stageId);
        if(stage && stage->audioType == "ov") {
          startInternal(*stage, store);
        }
      }
    }
    std::cout << "OK" << std::endl;
  } else {
    std::cout << "SKIPPED" << std::endl;
  }
}

void ov_controller_digitalstage_t::stop()
{
  std::lock_guard<std::recursive_mutex> lock(this->mutex);
  std::cout << "[ov_controller_digitalstage_t] stop() -> ";
  if(!isRunning) {
    stopInternal();
    isRunning = false;
    std::cout << "OK" << std::endl;
  } else {
    std::cout << "SKIPPED" << std::endl;
  }
}

void ov_controller_digitalstage_t::startInternal(const Stage& stage,
                                                 const Store* store)
{
  std::cout << "[ov_controller_digitalstage_t] startInternal()" << std::endl;
  if(stage.audioType == "ov") {
    // AUDIO BACKEND
    auto localDevice = store->getLocalDevice();
    if(localDevice && localDevice->soundCardId) {
      setSoundCard(*localDevice->soundCardId);
    }
    renderer->start_audiobackend();

    // THIS DEVICE
    syncLocalStageMember(store);

    // RELAY SERVER
    if(stage.ovIpv4 && stage.ovPort && stage.ovPin) {
      renderer->set_relay_server(*stage.ovIpv4, *stage.ovPort, *stage.ovPin);
    }

    // START SESSION
    renderer->start_session();
  }
}

void ov_controller_digitalstage_t::stopInternal()
{
  std::cout << "[ov_controller_digitalstage_t] stopInternal()" << std::endl;
  renderer->stop_audiobackend();
  renderer->clear_stage();
}

struct ReducedStageMember {
  double volume = 1;
  bool muted = true;
  std::string directivity = "omni";
  pos_t position = {0, 0, 0};
  zyx_euler_t orientation = {0, 0, 0};
};

ReducedStageMember reduceStageMember(const Store* store,
                                     const StageMember& stageMember)
{
  std::cout << "[ov_controller_digitalstage_t] reduceStageMember()"
            << std::endl;
  std::optional<const std::string> localDeviceId = store->getLocalDeviceId();
  if(!localDeviceId) {
    return {};
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

void ov_controller_digitalstage_t::syncLocalStageMember(const Store* store)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
#ifdef DEBUG
  std::cout << "[TRACE] Syncing local stage member" << std::endl;
#endif
  // Get local device and current stage member ID
  auto localDevice = store->getLocalDevice();
  auto stageMemberId = store->getStageMemberId();

  if(localDevice && stageMemberId) {
    // Local device is ready and user inside a stage
    auto stageMember = store->getStageMember(*stageMemberId);
    if(!stageMember) {
      std::cerr << "[ERROR] Could not find stage member " << *stageMemberId
                << " - "
                   "server side error?"
                << std::endl;
      return;
    }
    auto user = store->getUser(stageMember->userId);
    if(!user) {
      std::cerr << "[ERROR] Could not find user " << stageMember->userId
                << " - "
                   "server side error?"
                << std::endl;
      return;
    }

    ReducedStageMember reduced = reduceStageMember(store, *stageMember);

    auto tracks = store->getRemoteAudioTracksByStageMember(*stageMemberId);

    std::vector<device_channel_t> deviceChannels;
    // Now for all remote (but local) tracks
    for(const auto& track : tracks) {
      if(track.type == "ov" && track.ovSourcePort) {
#ifdef DEBUG
        std::cout << "[TRACE] Adding local track " << *track.ovSourcePort
                  << std::endl;
#endif
        // Look for custom track
        auto customTrackVolume =
            store->getCustomRemoteAudioTrackVolume(track._id);
        auto customTrackPosition =
            store->getCustomRemoteAudioTrackPosition(track._id);
        double volume =
            customTrackVolume ? customTrackVolume->volume : track.volume;
        pos_t pos{};
        if(customTrackPosition) {
          pos = {customTrackPosition->x, customTrackPosition->y,
                 customTrackPosition->z};
        } else {
          pos = {track.x, track.y, track.z};
        }
        std::string directivity = customTrackPosition
                                      ? customTrackPosition->directivity
                                      : track.directivity;
        deviceChannels.push_back({track._id,
                                  "system:capture_" + *track.ovSourcePort,
                                  volume, pos, directivity});
      }
    }

    if(stageMember->order == 255) {
      std::cerr << "OV not available" << std::endl;
      return;
    }
    double senderJitter =
        localDevice->ovSenderJitter ? *localDevice->ovSenderJitter : 5.;
    double receiverJitter =
        localDevice->ovSenderJitter ? *localDevice->ovSenderJitter : 5.;
    renderer->set_thisdev({
        stageMember->order, user->name, deviceChannels, reduced.position,
        reduced.orientation, reduced.volume, reduced.muted, senderJitter,
        receiverJitter,
        true // sendlocal always true?
    });
  }
}

void ov_controller_digitalstage_t::setSoundCard(const ID_TYPE& id)
{
  auto soundCard = client->getStore()->getSoundCard(id);
  if(soundCard) {
    audio_device_t audioDevice;
    audioDevice.srate = soundCard->sampleRate;
    audioDevice.periodsize = soundCard->periodSize;
    audioDevice.numperiods = soundCard->numPeriods;
    audioDevice.drivername = soundCard->driver ? *soundCard->driver : "jack";
    audioDevice.devicename = soundCard->uuid;
    renderer->configure_audio_backend(audioDevice);
  } else {
    std::cerr << "Could not find sound card " << id << std::endl;
  }
}