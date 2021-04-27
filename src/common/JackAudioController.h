#pragma once
#include <JuceHeader.h>
#include <jack/jack.h>
#include <thread>

class JackNotAvailableWindow : public juce::DocumentWindow {
public:
  JackNotAvailableWindow();

private:
  std::unique_ptr<juce::Label> label;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JackNotAvailableWindow)
};

class JackAudioController {
public:
  struct JackServerSettings {
    std::vector<std::string> inputPorts = {"system:capture_1"};
    std::vector<std::string> outputPorts = {"system:playback_1", "system:playback_2"};
    uint32_t sampleRate = 48000;
    uint32_t bufferSize = 123;
  };
  using ChangeListener = std::function<void(bool, const JackServerSettings&)>;

  JackAudioController();
  ~JackAudioController();

  [[nodiscard]] bool isAvailable() const;

  void setActive(bool active);

  [[nodiscard]] bool isActive() const;

  [[nodiscard]] std::vector<std::string> getInputPorts() const;
  [[nodiscard]] std::vector<std::string> getOutputPorts() const;
  [[nodiscard]] uint32_t getSampleRate() const;
  [[nodiscard]] uint32_t getBufferSize() const;

  void addListener(const ChangeListener& changeListener);

  void removeAllListeners();

private:
  void showDialog();
  void onAvailable();
  void onUnavailable();
  void startObservation();
  static void client_shutdown_cb(jack_status_t, const char*, void* args);
  void scanJackAudioSizes(jack_client_t* jackClient);
  void scanJackInputPorts(jack_client_t* jackClient);
  void scanJackOutputPorts(jack_client_t* jackClient);
  void observeJack();

private:
  bool isActive_;
  bool isAvailable_;
  bool shouldExit;
  std::vector<ChangeListener> changeListeners;
  std::unique_ptr<std::thread> observerThread;
  JackServerSettings serverSettings;

  // UI
  juce::DialogWindow::LaunchOptions dialogOptions;
  std::unique_ptr<JackNotAvailableWindow> window;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JackAudioController)
};