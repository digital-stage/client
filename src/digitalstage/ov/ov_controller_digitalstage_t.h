//
// Created by Tobias Hegemann on 28.04.21.
//

#ifndef OV_CONTROLLER_DIGITALSTAGE_T_H_
#define OV_CONTROLLER_DIGITALSTAGE_T_H_

#include <DigitalStage/Api/Client.h>
#include <DigitalStage/Api/Store.h>
#include <ov_render_tascar.h>
#include <ov_types.h>

class ov_controller_digitalstage_t {
public:
  ov_controller_digitalstage_t(ov_render_tascar_t* renderer_,
                               DigitalStage::Api::Client* client_);
  ~ov_controller_digitalstage_t();

  void start();
  void stop();

protected:
  void init();

  void handleStageJoined(const ID_TYPE& stageId, const ID_TYPE& groupId,
                         const DigitalStage::Api::Store* store);
  void handleStageLeft(const DigitalStage::Api::Store* store);
  void handleStageChanged(const ID_TYPE& stageId, const nlohmann::json& update,
                          const DigitalStage::Api::Store* store);


  void startInternal(const Stage& stage, const DigitalStage::Api::Store* store);
  void stopInternal();


  void setSoundCard(const ID_TYPE& id);
  void syncLocalStageMember(const DigitalStage::Api::Store* store);

private:
  bool isRunning;
  bool insideStage;
  ov_render_tascar_t* renderer;
  DigitalStage::Api::Client* client;
  mutable std::recursive_mutex mutex;
};

#endif // OV_CONTROLLER_DIGITALSTAGE_T_H_
