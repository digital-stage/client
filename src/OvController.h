#pragma once
#include "Client.h"
#include <JuceHeader.h>

//#include <ov_render_tascar.h>

class OvController {
public:
  OvController(DigitalStage::Client* client_);

  void init();

  void start();

  void stop();

private:
  DigitalStage::Client* client;
  // juce::Thread serviceThread;
};
