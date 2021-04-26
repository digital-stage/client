#include "ApplicationController.h"

ApplicationController::ApplicationController()
{
  init();
}

ApplicationController::~ApplicationController()
{
}

void ApplicationController::init()
{
  // Change working directory (many depends on it)
  getAppDataDir().setAsCurrentWorkingDirectory();

  jackAudioController.reset(new JackAudioController());
  jackAudioController->setActive(true);
  orlandoViolsClient.reset(new OrlandoViolsClient(jackAudioController.get()));

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  taskbar.reset(new TaskbarComponent());
  taskbar->onOpenStageClicked = []() {
    URL("https://box.orlandoviols.com").launchInDefaultBrowser();
  };
  taskbar->onOpenMixerClicked = []() {
    URL("http://localhost:8080").launchInDefaultBrowser();
  };
#endif

  orlandoViolsClient->start();
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

void ApplicationController::handleException(const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                         TRANS("error"), e.what());
}
