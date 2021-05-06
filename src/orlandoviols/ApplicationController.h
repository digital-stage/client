#pragma once

#include "OrlandoViolsClient.h"
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
#include "TaskbarComponent.h"
#endif
#include <JuceHeader.h>
#include <string>

class ApplicationController {

public:
  ApplicationController();
  ~ApplicationController();

private:
  void init();
  static juce::File getAppDataDir() ;
  static void handleException(const std::exception& e);

private:
  std::unique_ptr<JackAudioController> jackAudioController;
  std::unique_ptr<OrlandoViolsClient> orlandoViolsClient;

  // UI COMPONENTS
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  std::unique_ptr<TaskbarComponent> taskbar;
#endif
};