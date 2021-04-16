#pragma once
#include <JuceHeader.h>

#if JUCE_LINUX || JUCE_MAC
#include "ov_render_tascar.h"
#endif

class OvController {
public:
  OvController();
  ~OvController();

  void init();

  void start();

  void stop();

private:
  juce::Thread serviceThread;
}