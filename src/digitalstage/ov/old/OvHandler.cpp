#include "OvHandler.h"
#include <JuceHeader.h>
#include <eventpp/utilities/argumentadapter.h>

OvHandler::OvHandler(DigitalStage::Client* client_)
    : isRunning(false), client(client_)
{
  mixer.reset(new OvMixer());
  controller->addListener(
      [&](bool isAvailable, const std::vector<std::string>, const std::vector<std::string>) { handleJackAvailabilityChanged(isAvailable); });

  auto uuid = getmacaddr();
  renderer.reset(new ov_render_tascar_t(uuid, 0));
  renderer->set_runtime_folder(
      juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString() +
      "/");
  mixer.reset(new OvMixer());
}

void OvHandler::init()
{
  client->appendListener(
      DigitalStage::EventType::STAGE_JOINED,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventStageJoined&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventStageJoined& e,
                  const DigitalStage::Store& s) {
                auto stage = s.getStage(e.getStageId());
                auto group = s.getGroup(e.getGroupId());
                if(stage->audioType == "ov") {
                  isRunning = true;
                  // Create local audio tracks for all channels that shall be
                  // sent
                  auto localDevice = s.getLocalDevice();
                  if(localDevice) {
                    if(localDevice->soundCardId) {
                    }
                  } else {
                    std::cerr << "Local device is not ready" << std::endl;
                  }
                  // Consume all remote audio tracks
                  std::cout << "HANDLING NOW OV FOR STAGE " << stage->name
                            << " AND GROUP " << group->name << std::endl;
                  start();
                }
              })));

  client->appendListener(
      DigitalStage::EventType::STAGE_LEFT,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventStageLeft&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventStageLeft&,
                  const DigitalStage::Store&) { stop(); })));

  client->appendListener(
      DigitalStage::EventType::REMOTE_AUDIO_TRACK_ADDED,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventRemoteAudioTrackAdded&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventRemoteAudioTrackAdded&,
                  const DigitalStage::Store&) {
                std::cout << "TODO" << std::endl;
              })));

  client->appendListener(
      DigitalStage::EventType::REMOTE_AUDIO_TRACK_CHANGED,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventRemoteAudioTrackChanged&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventRemoteAudioTrackChanged&,
                  const DigitalStage::Store&) {
                std::cout << "TODO" << std::endl;
              })));

  client->appendListener(
      DigitalStage::EventType::REMOTE_AUDIO_TRACK_REMOVED,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventRemoteAudioTrackRemoved&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventRemoteAudioTrackRemoved&,
                  const DigitalStage::Store&) {
                std::cout << "TODO" << std::endl;
              })));

  client->appendListener(
      DigitalStage::EventType::LOCAL_DEVICE_READY,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventLocalDeviceReady&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventLocalDeviceReady&,
                  const DigitalStage::Store&) {

                std::cout << "SEND SOUNDCARD OF JACK" << std::endl;
              })));
}

void OvHandler::start()
{
  // TODO: Check if jack is running
  mixer->start();
  isRunning = true;
  if(jackAudioController->isAvailable()) {
    startOv();
  } else {
    jackAudioController->setActive(true);
  }
}

void OvHandler::stop()
{
  mixer->stop();
  isRunning = false;
}

struct ReducedStageMember {
  double volume = 1;
  bool muted = true;
  std::string directivity = "omni";
  pos_t position = {0, 0, 0};
  zyx_euler_t orientation = {0, 0, 0};
};

ReducedStageMember
reduceStageMember(const DigitalStage::Store& store,
                  const DigitalStage::stage_member_t& stageMember)
{
  std::optional<const std::string> localDeviceId = store.getLocalDeviceId();
  if(!localDeviceId) {
    return {};
  }
  auto group = store.getGroup(stageMember.groupId);
  auto customStageMemberVolume =
      store.getCustomStageMemberVolumeByStageMemberAndDevice(stageMember._id,
                                                             *localDeviceId);
  auto customStageMemberPosition =
      store.getCustomStageMemberPositionByStageMemberAndDevice(stageMember._id,
                                                               *localDeviceId);
  auto customGroupVolume = store.getCustomGroupVolumeByGroupAndDevice(
      stageMember.groupId, *localDeviceId);
  auto customGroupPosition = store.getCustomGroupPositionByGroupAndDevice(
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

void OvHandler::syncLocalStageMember(const DigitalStage::Store& store)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
#ifdef DEBUG
  std::cout << "[TRACE] Syncing local stage member" << std::endl;
#endif
  // Get local device and current stage member ID
  std::optional<const DigitalStage::device_t> localDevice =
      store.getLocalDevice();
  std::optional<const std::string> stageMemberId = store.getStageMemberId();

  if(localDevice && stageMemberId) {
    // Local device is ready and user inside a stage
    std::optional<const DigitalStage::stage_member_t> stageMember =
        store.getStageMember(*stageMemberId);
    if(!stageMember) {
      std::cerr << "[ERROR] Could not find stage member " << *stageMemberId
                << " - "
                   "server side error?"
                << std::endl;
      return;
    }
    std::optional<const DigitalStage::user_t> user =
        store.getUser(stageMember->userId);
    if(!user) {
      std::cerr << "[ERROR] Could not find user " << stageMember->userId
                << " - "
                   "server side error?"
                << std::endl;
      return;
    }

    ReducedStageMember reduced = reduceStageMember(store, *stageMember);

    const std::vector<DigitalStage::remote_audio_track_t> tracks =
        store.getRemoteAudioTracksByStageMember(*stageMemberId);

    std::vector<device_channel_t> deviceChannels;
    // Now for all remote (but local) tracks
    for(const auto& track : tracks) {
      if(track.type == "ov" && track.ovSourcePort) {
#ifdef DEBUG
        std::cout << "[TRACE] Adding local track " << *track.ovSourcePort
                  << std::endl;
#endif
        // Look for custom track
        std::optional<const DigitalStage::custom_remote_audio_track_volume_t>
            customTrackVolume =
                store.getCustomRemoteAudioTrackVolume(track._id);
        std::optional<const DigitalStage::custom_remote_audio_track_position_t>
            customTrackPosition =
                store.getCustomRemoteAudioTrackPosition(track._id);
        double volume =
            customTrackVolume ? customTrackVolume->volume : track.volume;
        pos_t pos;
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