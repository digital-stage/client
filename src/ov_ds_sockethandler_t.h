//
// Created by Tobias Hegemann on 01.05.21.
//

#ifndef OV_DS_SOCKETHANDLER_H_
#define OV_DS_SOCKETHANDLER_H_

#include <DigitalStage/Api/Client.h>
#include <DigitalStage/Types.h>
#include <nlohmann/json.hpp>
#include <ov_render_tascar.h>

class ov_ds_sockethandler_t {
public:
  ov_ds_sockethandler_t(ov_render_base_t* renderer_, DigitalStage::Api::Client* client_);
  ~ov_ds_sockethandler_t();

  void listen();

  void unlisten();

protected:
  void onReady(const DigitalStage::Api::Store* store);

  void onStageJoined(const DigitalStage::Types::ID_TYPE& stageId, const DigitalStage::Types::ID_TYPE& groupId, const DigitalStage::Api::Store* store);

  void handleStageJoined(const DigitalStage::Types::Stage& stage, const DigitalStage::Api::Store* store);

  void onStageChanged(const DigitalStage::Types::ID_TYPE& stageId, const nlohmann::json& update, const DigitalStage::Api::Store* store);

  void onStageLeft(const DigitalStage::Api::Store* store);

  void onDeviceChanged(const std::string& id, const nlohmann::json& update,
                       const DigitalStage::Api::Store* store);
  /**
   * This handler might create local audio tracks for all activated channels
   * @param soundCard
   * @param store
   */
  void onSoundCardAdded(const DigitalStage::Types::SoundCard& soundCard,
                            const DigitalStage::Api::Store* store);

  void setSoundCard(const DigitalStage::Types::SoundCard& soundCard,
                    const DigitalStage::Api::Store* store);
  /**
   * This handler will create local audio tracks for all activated channels and
   * remove deprecated local audio tracks for all now disabled channels.
   * @param id
   * @param update
   * @param store
   */
  void onSoundCardChanged(const DigitalStage::Types::ID_TYPE& id,
                              const nlohmann::json& update,
                              const DigitalStage::Api::Store* store);

  /**
   * This handler will sync TASCAR with the stage member of the added track.
   * Since TASCAR is not capable of small changes in sessions yet, the whole
   * stage member with all her/his tracks is replaced.
   * @param track
   * @param store
   */
  void onRemoteAudioTrackAdded(
      const DigitalStage::Types::AudioTrack& track,
      const DigitalStage::Api::Store* store);
  /**
   * This handler will sync TASCAR with the stage member of the added track.
   * Since TASCAR is not capable of small changes in sessions yet, the whole
   * stage member with all her/his tracks is replaced.
   * @param track
   * @param store
   */
  void onRemoteAudioTrackRemoved(
      const DigitalStage::Types::AudioTrack& track,
      const DigitalStage::Api::Store* store);

  // The following methods will respond to changes of positions and volumes
  // inside the stage and update all related TASCAR tracks (there are already
  // methods to simply update TASCAR instead of replacing the whole stage
  // member)
  void updateRemoteAudioTrackPosition(const DigitalStage::Types::ID_TYPE& id);
  void updateRemoteAudioTrackVolume(const DigitalStage::Types::ID_TYPE& id);
  /*
  void handleGroupChanged(const DigitalStage::Types::ID_TYPE& id,
                                const nlohmann::json& update,
                                const DigitalStage::Api::Store* store);
  void
  handleCustomGroupPositionAdded(const DigitalStage::Types::CustomGroupPosition&
  position, const DigitalStage::Api::Store* store); void
  handleCustomGroupPositionChanged(const DigitalStage::Types::ID_TYPE& id,
                                         const nlohmann::json& update,
                                         const DigitalStage::Api::Store* store);
  void
  handleCustomGroupPositionRemoved(const DigitalStage::Types::ID_TYPE& id,
                                         const DigitalStage::Api::Store* store);
  void
  handleCustomGroupVolumeAdded(const DigitalStage::Types::CustomGroupVolume&
  volume, const DigitalStage::Api::Store* store); void
  handleCustomGroupVolumeChanged(const DigitalStage::Types::ID_TYPE& id,
                                       const nlohmann::json& update,
                                       const DigitalStage::Api::Store* store);
  void
  handleCustomGroupVolumeRemoved(const DigitalStage::Types::ID_TYPE& id,
                                       const DigitalStage::Api::Store* store);
  void handleStageMemberChanged(const DigitalStage::Types::ID_TYPE& id,
                                const nlohmann::json& update,
                                const DigitalStage::Api::Store* store);
  void
  handleCustomStageMemberPositionAdded(const
  DigitalStage::Types::custom_stage_member_position_t& position, const
  DigitalStage::Api::Store* store); void
  handleCustomStageMemberPositionChanged(const DigitalStage::Types::ID_TYPE& id,
                                         const nlohmann::json& update,
                                         const DigitalStage::Api::Store* store);
  void
  handleCustomStageMemberPositionRemoved(const DigitalStage::Types::ID_TYPE& id,
                                         const DigitalStage::Api::Store* store);
  void
  handleCustomStageMemberVolumeAdded(const
  DigitalStage::Types::custom_stage_member_volume_t& volume, const
  DigitalStage::Api::Store* store); void
  handleCustomStageMemberVolumeChanged(const DigitalStage::Types::ID_TYPE& id,
                                         const nlohmann::json& update,
                                         const DigitalStage::Api::Store* store);
  void
  handleCustomStageMemberVolumeRemoved(const DigitalStage::Types::ID_TYPE& id,
                                         const DigitalStage::Api::Store* store);
  */

private:
  /**
   * Helper method to sync the input channels of the sound card with local audio
   * tracks. This will assure that there are local audio tracks only for the
   * current activated input channels.
   * @param soundCard
   */
  void syncInputChannels(DigitalStage::Types::SoundCard soundCard,
                         const DigitalStage::Api::Store* store);
  /**
   * Helper method to fully synchronize the given stage member with TASCAR.
   * Since tascar is not able to react dynamically to remote audio tracks,
   * we have to build the whole stage member each time a remote audio track is
   * added or removed - TODO: discuss, if maybe when changed, too?
   * @param stageMember
   * @param store
   */
  void syncStageMember(const DigitalStage::Types::ID_TYPE& id,
                       const DigitalStage::Api::Store* store);

  void syncWholeStage(const DigitalStage::Api::Store* store);

  ov_render_base_t* renderer;
  DigitalStage::Api::Client* client;
  bool insideOvStage;
};

#endif // OV_DS_SOCKETHANDLER_H_
