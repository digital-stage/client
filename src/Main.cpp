#include "../assets/utils.h"
#include "ApplicationState.h"
#include "ApplicationStore.h"
#include "AuthService.h"
#include "LoginPane.h"
#include "LoginWindow.h"
#include <JuceHeader.h>
#if JUCE_LINUX || JUCE_MAC
#include "OvController.h"
#endif

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

  void connect(const std::string& apiToken)
  {
    setApplicationState(ApplicationState::OUTSIDE_STAGE);
    nlohmann::json initialDevice;
    initialDevice["uuid"] = "123456";
    initialDevice["type"] = "ov";
    initialDevice["canAudio"] = true;
    initialDevice["canVideo"] = false;
    try {
      client->connect(apiToken, initialDevice);
    }
    catch(std::exception& e) {
      handleException(e);
    }
  }

  void handleException(const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                           TRANS("error"), e.what());
  }

  void disconnect()
  {
    try {
      client->disconnect();
    }
    catch(std::exception& e) {
      handleException(e);
    }
  }

  //==============================================================================
  void initialise(const juce::String& commandLine) override
  {
    juce::ignoreUnused(commandLine);

    // Show splashscreen first
    juce::SplashScreen* splashScreen = new juce::SplashScreen(
        ProjectInfo::projectName, getImageFromAssets("splash.png"), true);
    // juce::SplashScreen* splashScreen = new juce::SplashScreen(
    // ProjectInfo::projectName, getImageFromAssets("splash.png"), true);
    splashScreen->setVisible(true);

    // Init core
    client.reset(new DigitalStage::Client(API_URL));
#if JUCE_LINUX || JUCE_MAC
    ovController.reset(new OvController(client.get()));
    ovController
        ->init(); // This will start consuming events provided by the client
#endif

    // Init UI
    store.reset(new ApplicationStore(getApplicationName()));
    loginWindow.reset(new LoginWindow());
    loginPane.reset(new LoginPane(store->getUserSettings()));
    loginWindow->setResizeLimits(300, 400, 600, 1200);
    loginWindow->setContentOwned(loginPane.get(), true);

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
    taskbar->onSignOutClicked = [&]() {
      disconnect();
      setApplicationState(ApplicationState::SIGNED_OUT);
      loginWindow->setVisible(true);
    };
#else
    loginWindow->setVisible(true);
#endif

    loginPane->onSignedIn = [&](juce::String token) {
      loginWindow->setVisible(false);
      connect(token.toStdString());
    };

    if(!loginPane->signInWithStoredCredentials()) {
      loginWindow->setVisible(true);
    }

    splashScreen->deleteAfterDelay(juce::RelativeTime::seconds(2), false);
  }

  void shutdown() override
  {
    // loginWindow = nullptr;
    // loginPane = nullptr;
    // store = nullptr;
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    // taskbar = nullptr;
#endif
#if JUCE_LINUX || JUCE_MAC
    // ovController = nullptr;
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
  ApplicationState state = ApplicationState::SIGNED_OUT;
  std::unique_ptr<ApplicationStore> store;
  std::shared_ptr<DigitalStage::Client> client;

  // UI components
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  std::unique_ptr<TaskbarComponent> taskbar;
  std::unique_ptr<LoginWindow> loginWindow;
  std::unique_ptr<LoginPane> loginPane;
#endif
#if JUCE_LINUX || JUCE_MAC
  std::unique_ptr<OvController> ovController;
#endif
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(Main)