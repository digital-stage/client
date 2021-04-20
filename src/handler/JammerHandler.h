#pragma once
#include <Client.h>
#include <JuceHeader.h>

class JammerHandler {
public:
    JammerHandler(DigitalStage::Client* client_);

  void init();

private:
  DigitalStage::Client* client;
};