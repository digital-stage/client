#pragma once
#include <JuceHeader.h>
#include <jack/jack.h>
#include <thread>

class JackNotAvailableWindow : public juce::DocumentWindow {
public:
  explicit JackNotAvailableWindow()
      : juce::DocumentWindow(
            TRANS("JackUnvailable"),
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                ResizableWindow::backgroundColourId),
            juce::DocumentWindow::TitleBarButtons::minimiseButton)
  {
    label.reset(new juce::Label(
        "infoLabel",
        TRANS("Jack is not available. Please start it using QJackCtl.")));
    label->setSize(200, 100);
    setContentOwned(label.get(), true);
  }

private:
  std::unique_ptr<juce::Label> label;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JackNotAvailableWindow)
};

class JackAudioController {
  using ChangeListener = std::function<void(bool)>;

public:
  explicit JackAudioController()
      : isActive_(false), isAvailable_(false), shouldExit(false)
  {
    window.reset(new JackNotAvailableWindow());
  }
  ~JackAudioController()
  {
    shouldExit = true;
    // Wait for observer thread to end
    if(observerThread && observerThread->joinable()) {
      observerThread->join();
    }
  }

  inline bool isAvailable() const { return isAvailable_; }

  inline void setActive(bool active)
  {
    if(active != isActive_) {
      isActive_ = active;
      if(isActive_) {
        // Start observer thread
        shouldExit = false;
        startObservation();
      } else {
        shouldExit = true;
        window->setVisible(false);
      }
    }
  }

  inline bool isActive() const { return isActive_; }

  inline void addListener(ChangeListener changeListener)
  {
    changeListeners.push_back(changeListener);
  }

  inline void removeAllListeners() { changeListeners.clear(); }

private:
  inline void showDialog() {}
  inline void onAvailable()
  {
    isAvailable_ = true;
    juce::MessageManager::callAsync([&]() { window->setVisible(false); });
    for(auto& c : changeListeners) {
      c(isAvailable_);
    }
  }
  inline void onUnavailable()
  {
    isAvailable_ = false;
    juce::MessageManager::callAsync([&]() {
      if(!shouldExit) {
        window->setVisible(true);
      }
    });
    for(auto& c : changeListeners) {
      c(isAvailable_);
    }
  }
  inline void startObservation()
  {
    if(!shouldExit) {
      if(observerThread && observerThread->joinable()) {
        observerThread->join();
      }
      observerThread.reset(
          new std::thread(&JackAudioController::observeJack, this));
    }
  }
  inline static void client_shutdown_cb(jack_status_t, const char*, void* args)
  {
    auto instance = (JackAudioController*)args;
    instance->onUnavailable();
    // Restart observation again
    instance->startObservation();
  }
  inline void observeJack()
  {
    jack_options_t options = JackNoStartServer;
    jack_status_t status;
    jack_client_t* jackClient = jack_client_open("run_test", options, &status);
    if(!jackClient) {
      onUnavailable();
    }
    while(!shouldExit && jackClient == nullptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(400));
      jackClient = jack_client_open("run_test", options, &status);
    }
    jack_on_info_shutdown(jackClient, client_shutdown_cb, (void*)this);
    onAvailable();
  }

private:
  bool isActive_;
  bool isAvailable_;
  bool shouldExit;
  std::vector<ChangeListener> changeListeners;
  std::unique_ptr<std::thread> observerThread;

  // UI
  juce::DialogWindow::LaunchOptions dialogOptions;
  std::unique_ptr<JackNotAvailableWindow> window;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JackAudioController)
};