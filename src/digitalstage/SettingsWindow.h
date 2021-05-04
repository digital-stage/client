#pragma once
#include <JuceHeader.h>

class AudioSettingsPane : public juce::AudioDeviceSelectorComponent {
public:
  explicit AudioSettingsPane(juce::AudioDeviceManager& audioDeviceManager)
      : juce::AudioDeviceSelectorComponent(audioDeviceManager, 1, 256, 2, 2,
                                           false, false, true, true)
  {
    setSize(300, 200);
  }
};

class SettingsWindow : public juce::DocumentWindow {
public:
  explicit SettingsWindow(juce::AudioDeviceManager& audioDeviceManager)
      : juce::DocumentWindow(
            TRANS("Settings"),
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                ResizableWindow::backgroundColourId),
            juce::DocumentWindow::TitleBarButtons::closeButton),
        audioSettingsPane(new AudioSettingsPane(audioDeviceManager))
  {
    setUsingNativeTitleBar(true);
    setContentOwned(audioSettingsPane, true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    audioSettingsPane->setSize(300, 180);
    setResizable(false, false);
    centreWithSize(getWidth(), getHeight());
#endif
  }
  ~SettingsWindow() override { delete audioSettingsPane; }

protected:
  inline void closeButtonPressed() override { setVisible(false); }

private:
  AudioSettingsPane* audioSettingsPane;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};