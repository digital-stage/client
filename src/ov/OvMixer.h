#pragma once
#include <JuceHeader.h>
#include <thread>

class OvMixer {
public:
  OvMixer(const juce::File& appDataDir);
  ~OvMixer();

  void start();
  void stop();

protected:
  void downloadAndStart();

private:
  juce::File webmixer;
  std::unique_ptr<std::thread> downloadThread;
  std::unique_ptr<juce::ChildProcess> process;
  bool isInitializing;
  bool quitRequested;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OvMixer)
};