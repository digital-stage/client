#include "../../assets/utils.h"
#include "ApplicationController.h"
#include <JuceHeader.h>

#include <memory>

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
  bool moreThanOneInstanceAllowed() override { return false; }

  //==============================================================================
  void initialise(const juce::String& commandLine) override
  {
    juce::ignoreUnused(commandLine);

    // Show splashscreen first
    auto* splashScreen = new juce::SplashScreen(
        ProjectInfo::projectName, getImageFromAssets("digitalstage/splash.png"), true);
    splashScreen->setVisible(true);

    // Init controller
    controller = std::make_unique<ApplicationController>();

    // Get mac address
    auto addresses = juce::MACAddress::getAllAddresses();
    for(auto& address : addresses) {
      std::cout << address.toString("").toStdString() << std::endl;
    }
    
    splashScreen->deleteAfterDelay(juce::RelativeTime::seconds(2), false);
  }

  void shutdown() override
  {
    controller = nullptr;
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
  std::unique_ptr<ApplicationController> controller;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(Main)