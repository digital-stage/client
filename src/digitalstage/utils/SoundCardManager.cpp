#include "SoundCardManager.h"
#include "eventpp/utilities/argumentadapter.h"


SoundCardManager::SoundCardManager(
    DigitalStage::Api::Client* apiClient_,
    juce::AudioDeviceManager* audioDeviceManager_)
    : apiClient(apiClient_), audioDeviceManager(audioDeviceManager_)
{
  // Send soundcards when the local device is ready
  // apiClient->appendListener(DigitalStage::EventType::LOCAL_DEVICE_READY,
  // handleLocalDeviceReady);

}

SoundCardManager::~SoundCardManager()
{
  // Remove listener
  // apiClient->removeListener(handleLocalDeviceReady)
}