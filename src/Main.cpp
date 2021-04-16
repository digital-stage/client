#include "../assets/utils.h"
#include "ApplicationState.h"
#include "ApplicationStore.h"
#include "AuthService.h"
#include "LoginWindow.h"
#include <JuceHeader.h>

#ifndef SIGNUP_URL
#define SIGNUP_URL "https://live.digital-stage.org/auth/signup"
#endif

#ifndef STAGE_URL
#define STAGE_URL "https://live.digital-stage.org/stage"
#endif

#ifndef MIXER_URL
#define MIXER_URL "https://live.digital-stage.org/mixer"
#endif

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
#include "TaskbarComponent.h"
#endif

//==============================================================================
class Main : public juce::JUCEApplication {
public:
  //==============================================================================
  Main() {}

  // We inject these as compile definitions from the CMakeLists.txt
  // If you've enabled the juce header with
  // `juce_generate_juce_header(<thisTarget>)` you could `#include
  // <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
  const juce::String getApplicationName() override
  {
    return JUCE_APPLICATION_NAME_STRING;
  }
  const juce::String getApplicationVersion() override
  {
    return JUCE_APPLICATION_VERSION_STRING;
  }
  bool moreThanOneInstanceAllowed() override { return true; }

  void setApplicationState(ApplicationState value)
  {
    state = value;
    taskbar->setApplicationState(state);
  }

  //==============================================================================
  void initialise(const juce::String& commandLine) override
  {
    juce::ignoreUnused(commandLine);

    // Show splashscreen first
    juce::SplashScreen* splashScreen = new juce::SplashScreen(
        ProjectInfo::projectName, getImageFromAssets("splash.png"), true);
    splashScreen->setVisible(true);

    // Init UI
    authService.reset(new DigitalStage::AuthService(AUTH_URL));
    loginWindow.reset(new LoginWindow());
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    taskbar.reset(new TaskbarComponent(state));
    taskbar->onOpenStageClicked = []() {
      URL(STAGE_URL).launchInDefaultBrowser();
    };
    taskbar->onOpenMixerClicked = []() {
      URL(MIXER_URL).launchInDefaultBrowser();
    };
    taskbar->onSignUpClicked = []() {
      URL(SIGNUP_URL).launchInDefaultBrowser();
    };
    taskbar->onSignInClicked = [&]() { loginWindow->setVisible(true); };
    taskbar->onSignOutClicked = [&]() { loginWindow->setVisible(true); };
#else
    loginWindow->setVisible(true);
#endif

    // Check if user is signed in
    store.reset(new ApplicationStore(getApplicationName()));
    const juce::String token = store->getUserSettings()->getValue("token", "");
    if(token.length() > 0 &&
       authService->verifyTokenSync(token.toStdString())) {
      // Signed in
    } else {
      // Show login panel
      loginWindow->setVisible(true);
    }

    loginWindow->onSubmit = [&](juce::String email, juce::String password) {
      const auto requestedToken =
          authService->signInSync(email.toStdString(), password.toStdString());
      if(token.length() > 0) {
        store->getUserSettings()->setValue("token",
                                           juce::String(requestedToken));
      }
    };

    splashScreen->deleteAfterDelay(juce::RelativeTime::seconds(2), false);
  }

  void shutdown() override
  {
    loginWindow = nullptr;
    store = nullptr;
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    taskbar = nullptr;
#endif
  }

  //==============================================================================
  void systemRequestedQuit() override
  {
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app
    // to close.
    quit();
  }

  void anotherInstanceStarted(const juce::String& commandLine) override
  {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
    juce::ignoreUnused(commandLine);
  }

private:
  // Business logic parts
  std::unique_ptr<DigitalStage::AuthService> authService;
  ApplicationState state = ApplicationState::SIGNED_OUT;
  std::unique_ptr<ApplicationStore> store;

  // UI components
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  std::unique_ptr<TaskbarComponent> taskbar;
  std::unique_ptr<LoginWindow> loginWindow;
#endif
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(Main)