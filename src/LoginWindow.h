#pragma once
#include <JuceHeader.h>

class LoginWindow : public juce::DocumentWindow {
public:
  LoginWindow();

protected:
  void closeButtonPressed() override;
  // juce::TextEditor emailEditor;
  // juce::TextEditor passwordEditor;
  // juce::TextButton loginButton;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginWindow)
};