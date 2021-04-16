#pragma once
#include <JuceHeader.h>

class LoginWindow : public juce::DocumentWindow {
  class LoginPane : public juce::Component, public juce::Button::Listener {
  public:
    LoginPane(int minWidth, int minHeight);
    ~LoginPane() override;
    std::function<void(juce::String, juce::String)> onSubmit;

  protected:
    void resized() override;
    void buttonClicked(juce::Button* button) override;

  private:
    int minWidth;
    int minHeight;
    juce::ImageComponent logo;
    juce::FlexBox container;
    juce::Label emailLabel;
    juce::TextEditor emailEditor;
    juce::Label passwordLabel;
    juce::TextEditor passwordEditor;
    juce::TextButton loginButton;
  };

public:
  LoginWindow();
  std::function<void(juce::String, juce::String)> onSubmit;

protected:
  void closeButtonPressed() override;
  std::unique_ptr<LoginPane> pane;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginWindow)
};