#include "ApplicationController.h"
#include <eventpp/utilities/argumentadapter.h>

ApplicationController::ApplicationController()
{
  init();
}

ApplicationController::~ApplicationController()
{
  signOut();
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

void ApplicationController::init()
{
  // Change working directory (many depends on it)
  getAppDataDir().setAsCurrentWorkingDirectory();

  apiClient.reset(new DigitalStage::Api::Client(API_URL));
  audioDeviceManager.reset(new juce::AudioDeviceManager());

  soundCardManager.reset(
      new SoundCardManager(apiClient.get(), audioDeviceManager.get()));

  store.reset(new ApplicationStore(ProjectInfo::projectName));
  settingsWindow.reset(new SettingsWindow(*audioDeviceManager));
  loginWindow.reset(new LoginWindow());
  loginPane.reset(new LoginPane());
  loginWindow->setResizeLimits(300, 400, 600, 1200);
  loginWindow->setContentOwned(loginPane.get(), true);
#if JUCE_LINUX || JUCE_MAC
  jackAudioController.reset(new JackAudioController());
  jackAudioController->setActive(true);
  ovHandler.reset(new OvHandler(jackAudioController.get(), apiClient.get()));
  ovHandler->init(); // This will start consuming events provided by the client
#endif

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  taskbar.reset(new TaskbarComponent(ApplicationState::SIGNED_OUT));
  taskbar->onOpenStageClicked = []() {
    URL(STAGE_URL).launchInDefaultBrowser();
  };
  taskbar->onOpenMixerClicked = []() {
    URL(MIXER_URL).launchInDefaultBrowser();
  };
  taskbar->onSignUpClicked = []() { URL(SIGNUP_URL).launchInDefaultBrowser(); };
  taskbar->onSignOutClicked = [&]() { signOut(); };
  taskbar->onSettingsClicked = [&]() { settingsWindow->setVisible(true); };
#endif
  loginPane->onSignedIn = [&](juce::String token) {
    loginWindow->setVisible(false);
    handleSignIn(token);
  };

  const juce::String token = store->getUserSettings()->getValue("token", "");
  if(token.length() > 0) {
    handleSignIn(token);
  } else {
    loginWindow->setVisible(true);
  }
}

const juce::File ApplicationController::getAppDataDir() const
{
  const juce::File folder =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
#if JUCE_MAC
          .getChildFile("Application Support")
#endif
          .getChildFile(ProjectInfo::projectName);
  if(!folder.exists()) {
    folder.createDirectory();
  } else if(!folder.isDirectory()) {
    throw new std::runtime_error("Could not create application data folder, "
                                 "since it already exists as file");
  }
  return folder;
}

void ApplicationController::handleSignIn(const juce::String token)
{
  nlohmann::json initialDevice;
  initialDevice["uuid"] = "123456";
  initialDevice["type"] = "ov";
  initialDevice["canAudio"] = true;
  initialDevice["canVideo"] = false;
  // Get
  try {
    taskbar->setApplicationState(ApplicationState::OUTSIDE_STAGE);
    apiClient->connect(token.toStdString(), initialDevice);
    store->getUserSettings()->setValue("token", token);
    store->getUserSettings()->save();
  }
  catch(std::exception& e) {
    handleException(e);
    signOut();
  }
}
void ApplicationController::signOut()
{
  apiClient->disconnect();
  store->getUserSettings()->removeValue("token");
  store->getUserSettings()->save();
  taskbar->setApplicationState(ApplicationState::SIGNED_OUT);
}

void ApplicationController::handleException(const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                         TRANS("error"), e.what());
}
