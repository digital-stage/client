#pragma once
#include <JuceHeader.h>
#include <DigitalStage/Api/Client.h>

class JammerHandler {
public:
  JammerHandler(DigitalStage::Api::Client* client_);

  void init();

private:
  DigitalStage::Api::Client* client;
};