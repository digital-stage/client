#pragma once
#include <JuceHeader.h>

class LoginWindow : public juce::DocumentWindow {
public:
  explicit LoginWindow()
      : juce::DocumentWindow(
            TRANS("Login"),
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                ResizableWindow::backgroundColourId),
            juce::DocumentWindow::TitleBarButtons::allButtons)
  {
    setUsingNativeTitleBar(true);
#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
#endif
  }

protected:
  inline void closeButtonPressed() override
  {
    JUCEApplication::getInstance()->systemRequestedQuit();
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginWindow)
};