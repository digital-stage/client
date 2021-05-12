//
// Created by Tobias Hegemann on 27.04.21.
//

#ifndef OVHANDLER_H_
#define OVHANDLER_H_

#include "../../common/JackAudioController.h"
#include "../../common/OvMixer.h"
#include <DigitalStage/Api/Client.h>
#include <ov_ds_sockethandler_t.h>
#include <ov_render_tascar.h>

using namespace DigitalStage::Api;

class OvHandler {
public:
  OvHandler(JackAudioController* controller, Client* client_);
  ~OvHandler();

protected:
  void onReady(const Store* store);
  void
  handleJackChanged(bool isAvailable,
                    const JackAudioController::JackServerSettings& settings);

private:
  void sendSoundCard();
  void start();
  void stop();

  JackAudioController* jackAudioController;
  Client* client;
  std::unique_ptr<ov_ds_sockethandler_t> controller;
  std::unique_ptr<ov_render_tascar_t> renderer;
  std::unique_ptr<OvMixer> mixer;
  bool isRunning;
};

#endif // OVHANDLER_H_
