//
// Created by Tobias Hegemann on 27.04.21.
//

#ifndef OVHANDLER_H_
#define OVHANDLER_H_

#include "../../common/JackAudioController.h"
#include "../../common/OvMixer.h"
#include "ov_client.h"
#include <DigitalStage/Api/Client.h>
#include <ov_render_tascar.h>

using namespace DigitalStage::Api;

class OvHandler {
public:
  OvHandler(JackAudioController* controller, Client* client_);

  void init();

private:
  void handleReady(const Store* store);
  void
  handleJackChanged(bool isAvailable,
                    const JackAudioController::JackServerSettings& settings);
  JackAudioController* jackAudioController;
  Client* client;
  std::unique_ptr<ov_client> controller;
  std::unique_ptr<ov_render_tascar_t> renderer;
  std::unique_ptr<OvMixer> mixer;
  bool isRunning;
};

#endif // OVHANDLER_H_
