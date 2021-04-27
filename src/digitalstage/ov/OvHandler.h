//
// Created by Tobias Hegemann on 27.04.21.
//

#ifndef OVHANDLER_H_
#define OVHANDLER_H_

#include "../../common/JackAudioController.h"
#include <DigitalStage/Api/Client.h>
#include "../../common/OvMixer.h"

using namespace DigitalStage::Api;

class OvHandler {
public:
  OvHandler(JackAudioController* controller, Client* client_);
  ~OvHandler();

  void init();

private:
  void handleJackChanged(bool isAvailable, const JackAudioController::JackServerSettings& settings);
  JackAudioController* controller;
  Client* client;
  std::unique_ptr<OvMixer> mixer;
};

#endif // OVHANDLER_H_
