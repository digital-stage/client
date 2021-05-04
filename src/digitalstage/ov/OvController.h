//
// Created by Tobias Hegemann on 28.04.21.
//

#ifndef OV_OVCONTROLLER_H_
#define OV_OVCONTROLLER_H_

#include <ov_render_tascar.h>
#include <DigitalStage/Api/Client.h>

struct ReducedStageMember {
  double volume = 1;
  bool muted = true;
  std::string directivity = "omni";
  pos_t position = {0, 0, 0};
  zyx_euler_t orientation = {0, 0, 0};
};

class OvController {
public:
  OvController(ov_render_tascar_t* renderer_,
  DigitalStage::Api::Client* client_);
  ~OvController();

  void start();
  void stop();

protected:
  void init();

private:
  void startService();
  void stopService();

  void updateSoundCard(soundcard_t soundCard);
  void updateLocalStageMember();

  bool hasSoundCard();
  DigitalStage::Types::Device getLocalDevice() noexcept(false);
  DigitalStage::Types::soundcard_t getSoundCard(const ID_TYPE& id) noexcept(false);
  DigitalStage::Types::soundcard_t getCurrentSoundCard() noexcept(false);
  std::pair<DigitalStage::Types::StageMember, DigitalStage::Types::user_t> getLocalStageMember() noexcept(false);
  std::pair<DigitalStage::Types::StageMember, DigitalStage::Types::user_t> getStageMember(const ID_TYPE& id) noexcept(false);
  ReducedStageMember reduceStageMember(const DigitalStage::Types::StageMember& stageMember) noexcept(false);

  //bool isReady;
  bool isRunning;
  bool shouldStart;
  DigitalStage::Api::Client* client;
  ov_render_tascar_t* renderer;
};

#endif // OV_OVCONTROLLER_H_
