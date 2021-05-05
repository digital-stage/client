#include "ApplicationController.h"

#include <memory>

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

  apiClient = std::make_shared<DigitalStage::Api::Client>(API_URL);
  audioDeviceManager = std::make_unique<juce::AudioDeviceManager>();

  soundCardManager = std::make_unique<SoundCardManager>(
      apiClient.get(), audioDeviceManager.get());

  store = std::make_unique<ApplicationStore>(ProjectInfo::projectName);
  settingsWindow = std::make_unique<SettingsWindow>(*audioDeviceManager);
  loginWindow = std::make_unique<LoginWindow>();
  loginPane = std::make_unique<LoginPane>();
  loginWindow->setResizeLimits(300, 400, 600, 1200);
  loginWindow->setContentOwned(loginPane.get(), true);
#if JUCE_LINUX || JUCE_MAC
  jackAudioController = std::make_unique<JackAudioController>();
  jackAudioController->setActive(true);
  ovHandler = std::make_unique<OvHandler>(jackAudioController.get(), apiClient.get());
  ovHandler->init(); // This will start consuming events provided by the client
#endif

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  taskbar = std::make_unique<TaskbarComponent>(ApplicationState::SIGNED_OUT);
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
  loginPane->onSignedIn = [&](const juce::String& token) {
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

juce::File ApplicationController::getAppDataDir()
{
  juce::File folder =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
#if JUCE_MAC
          .getChildFile("Application Support")
#endif
          .getChildFile(ProjectInfo::projectName);
  if(!folder.exists()) {
    folder.createDirectory();
  } else if(!folder.isDirectory()) {
    throw std::runtime_error("Could not create application data folder, "
                                 "since it already exists as file");
  }
  return folder;
}

void ApplicationController::handleSignIn(const juce::String& token)
{
  nlohmann::json initialDevice;
  initialDevice["uuid"] = "123456";
  initialDevice["type"] = "ov";
  initialDevice["canAudio"] = true;
  initialDevice["canVideo"] = false;
  initialDevice["sendAudio"] = true;
  initialDevice["receiveAudio"] = true;
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
