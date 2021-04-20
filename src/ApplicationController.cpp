#include "ApplicationController.h"
#include "eventpp/utilities/argumentadapter.h"

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
  // orlandoViolsController = nullptr;
#endif
}

void ApplicationController::init()
{
  const juce::File appDataDir = getAppDataDir();

  apiClient.reset(new DigitalStage::Client(API_URL));
  // Send soundcards when the local device is ready
  apiClient->appendListener(
      DigitalStage::EventType::LOCAL_DEVICE_READY,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventLocalDeviceReady&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventLocalDeviceReady& e,
                  const DigitalStage::Store& s) {

              })));

  audioDeviceManager.reset(new juce::AudioDeviceManager());
  store.reset(new ApplicationStore(ProjectInfo::projectName));
  settingsWindow.reset(new SettingsWindow(*audioDeviceManager));
  loginWindow.reset(new LoginWindow());
  loginPane.reset(new LoginPane());
  loginWindow->setResizeLimits(300, 400, 600, 1200);
  loginWindow->setContentOwned(loginPane.get(), true);
#if JUCE_LINUX || JUCE_MAC
  ovHandler.reset(new OvHandler(apiClient.get(), appDataDir));
  ovHandler->init(); // This will start consuming events provided by the client
  orlandoViolsClient.reset(
      new OrlandoViolsClient(appDataDir.getFullPathName().toStdString()));
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
#if JUCE_LINUX || JUCE_MAC
  // On unix systems the user can switch between orlandoviols and digitalstage
  taskbar->onUseDigitalStageClicked = [&]() { switchToDigitalStage(); };
  taskbar->onUseOrlandoViolsClicked = [&]() { switchToOrlandoViols(); };
#endif
#else
  loginWindow->setVisible(true);
#endif

  loginPane->onSignedIn = [&](juce::String token) {
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    loginWindow->setVisible(false);
#endif
    signIn(token);
  };

  const juce::String token = store->getUserSettings()->getValue("token", "");
  if(token.length() > 0) {
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    loginWindow->setVisible(true);
#endif
    signIn(token);
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

void ApplicationController::signIn(const juce::String token)
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

#if JUCE_LINUX || JUCE_MAC
void ApplicationController::switchToDigitalStage()
{
  orlandoViolsClient->stop();
  loginWindow->setVisible(true);
  taskbar->setApplicationState(ApplicationState::SIGNED_OUT);
}
void ApplicationController::switchToOrlandoViols()
{
  loginWindow->setVisible(false);
  orlandoViolsClient->start();
  taskbar->setApplicationState(ApplicationState::ORLANDOVIOLS_STANDALONE);
}
#endif

void ApplicationController::handleException(const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                         TRANS("error"), e.what());
}
