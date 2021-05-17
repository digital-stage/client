#include "ApplicationController.h"
#include "../digitalstage/ApplicationController.h"

#include <memory>

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

  jackAudioController = std::make_unique<JackAudioController>();
  jackAudioController->setActive(true);
  orlandoViolsClient = std::make_unique<OrlandoViolsClient>(jackAudioController.get());

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  taskbar = std::make_unique<TaskbarComponent>();
  taskbar->onOpenStageClicked = []() {
    URL("https://box.orlandoviols.com").launchInDefaultBrowser();
  };
  taskbar->onOpenMixerClicked = []() {
    URL("http://localhost:8080").launchInDefaultBrowser();
  };
#endif

  orlandoViolsClient->start();
}

juce::File ApplicationController::getAppDataDir()
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
    throw std::runtime_error("Could not create application data folder, "
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
juce::String ApplicationController::getMacAddress() {
  auto macAddresses = juce::MACAddress::getAllAddresses();
  for(auto& macAddress : macAddresses) {
    std::cout << macAddress.toString().toStdString() << std::endl;
  }
  return macAddresses[0];
}
