#pragma once
#include <JuceHeader.h>
#include <DigitalStage/Api/Client.h>

class SoundCardManager {

public:
  SoundCardManager(DigitalStage::Api::Client* apiClient_,
                   juce::AudioDeviceManager* audioDeviceManager_);
  ~SoundCardManager();

private:
#ifdef __clang__
  __unused
#endif
      DigitalStage::Api::Client* apiClient;
#ifdef __clang__
  __unused
#endif
      juce::AudioDeviceManager* audioDeviceManager;
};