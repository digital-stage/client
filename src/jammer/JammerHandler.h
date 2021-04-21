#pragma once
#include <JuceHeader.h>
#include <ds/Client.h>

class JammerHandler {
public:
  JammerHandler(DigitalStage::Client* client_);

  void init();

private:
  DigitalStage::Client* client;
};