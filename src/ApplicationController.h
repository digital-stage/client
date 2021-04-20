#pragma once

#include "ApplicationStore.h"
#include "handler/JammerHandler.h"
#if JUCE_LINUX || JUCE_MAC
#include "OrlandoViolsClient.h"
#include "handler/OvHandler.h"
#endif
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
#include "TaskbarComponent.h"
#endif
#include "LoginPane.h"
#include "LoginWindow.h"
#include "SettingsWindow.h"
#include <Client.h>
#include <JuceHeader.h>
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
  const juce::File getAppDataDir() const;
  void signIn(const juce::String token);
  void signOut();
#if JUCE_LINUX || JUCE_MAC
  void switchToOrlandoViols();
  void switchToDigitalStage();
#endif
  void handleException(const std::exception& e);

private:
  std::unique_ptr<juce::AudioDeviceManager> audioDeviceManager;
  // Store to save api token and other usefull informations
  std::unique_ptr<ApplicationStore> store;
  // API client for receiving/sending events from the Digital Stage API server
  std::shared_ptr<DigitalStage::Client> apiClient;
  // Event handler for jammer
  std::unique_ptr<JammerHandler> jammerHandler;
#if JUCE_LINUX || JUCE_MAC
  // Event handler for OV
  std::unique_ptr<OvHandler> ovHandler;
  // Client to use the orlandoviols frontend (without Digital Stage)
  std::unique_ptr<OrlandoViolsClient> orlandoViolsClient;
#endif

  // UI COMPONENTS
  std::unique_ptr<LoginWindow> loginWindow;
  std::unique_ptr<LoginPane> loginPane;
  std::unique_ptr<SettingsWindow> settingsWindow;
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
  std::unique_ptr<TaskbarComponent> taskbar;
#endif
};