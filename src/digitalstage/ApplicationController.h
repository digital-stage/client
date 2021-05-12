#pragma once

#include "ApplicationStore.h"
#include "jammer/JammerHandler.h"
#include "utils/SoundCardManager.h"
#if JUCE_LINUX || JUCE_MAC
#include "ov/OvHandler.h"
#include "../common/JackAudioController.h"
#endif
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
#include "TaskbarComponent.h"
#endif
#include "LoginPane.h"
#include "LoginWindow.h"
#include "SettingsWindow.h"
#include <JuceHeader.h>
#include <DigitalStage/Api/Client.h>
#include <string>

#ifndef SIGNUP_URL
#define SIGNUP_URL "https://live.digital-stage.org/auth/signup"
#endif

#ifndef STAGE_URL
#define STAGE_URL "https://live.digital-stage.org/stage"
#endif

#ifndef MIXER_URL
#define MIXER_URL "https://live.digital-stage.org/mixer"
#endif

class ApplicationController {

public:
  ApplicationController();
  ~ApplicationController();

private:
  void init();
  static juce::File getAppDataDir() ;
  void handleSignIn(const juce::String& token);
  void signOut();
  static void handleException(const std::exception& e);
  static juce::String getMacAddress();

private:
  std::unique_ptr<juce::AudioDeviceManager> audioDeviceManager;
  // Store to save api token and other usefull informations
  std::unique_ptr<ApplicationStore> store;
  // API client for receiving/sending events from the Digital Stage API server
  std::shared_ptr<DigitalStage::Api::Client> apiClient;
  std::unique_ptr<SoundCardManager> soundCardManager;
  // Event handler for jammer
  std::unique_ptr<JammerHandler> jammerHandler;
#if JUCE_LINUX || JUCE_MAC
  std::unique_ptr<JackAudioController> jackAudioController;
  // Event handler for OV
  std::unique_ptr<OvHandler> ovHandler;
#endif

  // UI COMPONENTS
  std::unique_ptr<LoginWindow> loginWindow;
  std::unique_ptr<LoginPane> loginPane;
  std::unique_ptr<SettingsWindow> settingsWindow;
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  std::unique_ptr<TaskbarComponent> taskbar;
#endif
};