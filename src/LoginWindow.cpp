#include "LoginWindow.h"

LoginWindow::LoginWindow()
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
  // loginButton.setButtonText(TRANS("login"));
  // this->addAndMakeVisible(emailEditor);
  // this->addAndMakeVisible(passwordEditor);
  // this->addAndMakeVisible(loginButton);

  setSize(500, 300);
}

void LoginWindow::closeButtonPressed()
{
  // This is called when the user tries to close this window. Here, we'll just
  // ask the app to quit when this happens, but you can change this to do
  // whatever you need.
  JUCEApplication::getInstance()->systemRequestedQuit();
}
