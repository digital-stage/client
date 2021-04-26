#pragma once
#include <JuceHeader.h>
#include <ds/Client.h>

class SoundCardManager {

public:
  SoundCardManager(DigitalStage::Client* apiClient_,
                   juce::AudioDeviceManager* audioDeviceManager_);
  ~SoundCardManager();

protected:
  void handleLocalDeviceReady(const DigitalStage::EventLocalDeviceReady&,
                              const DigitalStage::Store&);

private:
  __unused DigitalStage::Client* apiClient;
  __unused juce::AudioDeviceManager* audioDeviceManager;
};