#pragma once
#include <AuthService.h>
#include <JuceHeader.h>

class LoginPane : public juce::Component, public juce::Button::Listener {
public:
  LoginPane();
  ~LoginPane() override;

  void signOut();

  // juce::String getToken() const;

protected:
  void resized() override;
  void buttonClicked(juce::Button* button) override;

public:
  std::function<void(juce::String)> onSignedIn;

private:
  std::unique_ptr<DigitalStage::AuthService> authService;
  juce::ImageComponent logo;
  juce::FlexBox container;
  juce::Label errorLabel;
  juce::Label emailLabel;
  juce::TextEditor emailEditor;
  juce::Label passwordLabel;
  juce::TextEditor passwordEditor;
  juce::TextButton loginButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginPane)
};