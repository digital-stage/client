#include "ApplicationController.h"


ApplicationController::ApplicationController() : state(ApplicationState::SIGNED_OUT) {
    init();
}

ApplicationController::~ApplicationController() {
    signOut();
    // loginWindow = nullptr;
    // loginPane = nullptr;
    // store = nullptr;
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    // taskbar = nullptr;
#endif
#if JUCE_LINUX || JUCE_MAC
    // ovController = nullptr;
    // orlandoViolsController = nullptr;
#endif
}

void ApplicationController::init() {
    apiClient.reset(new DigitalStage::Client(API_URL));
#if JUCE_LINUX || JUCE_MAC
    ovHandler.reset(new OvHandler(apiClient.get()));
    ovHandler->init(); // This will start consuming events provided by the client
        orlandoViolsClient.reset(new OrlandoViolsClient());
#endif

    // Init UI
    store.reset(new ApplicationStore(ProjectInfo::projectName));
    loginWindow.reset(new LoginWindow());
    loginPane.reset(new LoginPane());
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
      signOut();
    };
    #if JUCE_LINUX || JUCE_MAC
    // On unix systems the user can switch between orlandoviols and digitalstage
    taskbar->onUseDigitalStageClicked = [&]() {
      switchToDigitalStage();
    };
    taskbar->onUseOrlandoViolsClicked = [&]() {
      switchToOrlandoViols();
    };
    #endif
#else
    loginWindow->setVisible(true);
#endif

    loginPane->onSignedIn = [&](juce::String token) {
      loginWindow->setVisible(false);
      signIn(token);
    };

    const juce::String token = store->getUserSettings()->getValue("token", "");
    if(token.length() > 0) {
      loginWindow->setVisible(true);
      signIn(token);
  }
}

void ApplicationController::signIn(const juce::String token) {
  nlohmann::json initialDevice;
  initialDevice["uuid"] = "123456";
  initialDevice["type"] = "ov";
  initialDevice["canAudio"] = true;
  initialDevice["canVideo"] = false;
  try {
    state = ApplicationState::OUTSIDE_STAGE;
      apiClient->connect(token.toStdString(), initialDevice);
      store->getUserSettings()->setValue("token", token);
      store->getUserSettings()->save();
  }
  catch(std::exception& e) {
      handleException(e);
      signOut();
} 
}
void ApplicationController::signOut() {
    apiClient->disconnect();
      store->getUserSettings()->removeValue("token");
      store->getUserSettings()->save();
  state = ApplicationState::SIGNED_OUT;
} 


    #if JUCE_LINUX || JUCE_MAC

void ApplicationController::switchToDigitalStage() {
  orlandoViolsClient->stop();
  loginPane->setVisible(true);
} 
void ApplicationController::switchToOrlandoViols() {
  loginPane->setVisible(false);
  orlandoViolsClient->start();
} 
    #endif

void ApplicationController::handleException(const std::exception& e)
{
std::cerr << e.what() << std::endl;
juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                        TRANS("error"), e.what());
}
