#pragma once
#include "../common/JackAudioController.h"
#include "../common/OvMixer.h"
#include <JuceHeader.h>
#include <jack/jack.h>

#include <ov_client_orlandoviols.h>
#include <ov_render_tascar.h>

#include <memory>

#ifndef ORLANDOVIOLS_FRONTEND_URL
#define ORLANDOVIOLS_FRONTEND_URL "http://oldbox.orlandoviols.com"
#endif

class OrlandoViolsClient {
public:
  explicit OrlandoViolsClient(JackAudioController* controller)
      :  shouldRun(false), isRunning(false), jackAudioController(controller)
  {
    mixer = std::make_unique<OvMixer>();
    controller->addListener(
        [&](bool isAvailable, const JackAudioController::JackServerSettings&) {
          handleJackAvailabilityChanged(isAvailable);
        });
  }

  inline void handleJackAvailabilityChanged(bool isAvailable)
  {
    if(isAvailable) {
      if(shouldRun && !isRunning) {
        startOv();
      }
    } else {
      if(isRunning) {
        stopOv();
      }
    }
  }

  ~OrlandoViolsClient() { stop(); }

  inline void start()
  {
    shouldRun = true;
    mixer->start();
    if(jackAudioController->isAvailable()) {
      startOv();
    } else {
      jackAudioController->setActive(true);
    }
  }

  inline void stop()
  {
    shouldRun = false;
    stopOv();
  }

private:
  inline void startOv()
  {
    renderer = std::make_unique<ov_render_tascar_t>(getmacaddr(), 0);
    client = std::make_unique<ov_client_orlandoviols_t>(
        *renderer, ORLANDOVIOLS_FRONTEND_URL);
    const std::string workingFolderPath =
        juce::File::getCurrentWorkingDirectory()
            .getFullPathName()
            .toStdString() +
        "/";
    const juce::File zitaRootFolder =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::currentExecutableFile)
            .getParentDirectory();
    renderer->set_zita_path(zitaRootFolder.getFullPathName().toStdString() +
                            "/");
    renderer->set_runtime_folder(workingFolderPath);
    client->set_runtime_folder(workingFolderPath);
    client->start_service();
    isRunning = true;
  }

  inline void stopOv()
  {
    if(client) {
      // TODO: The current orlandoviols client is waiting for the thread to
      // finish - but disonnection to the server could lead to a very long
      // timeout, so we might have to use a shutdown thread or kill the
      // corresponding thread directly..
      // killThread.reset(new std::thread(&OrlandoViolsClient::killClient,
      // this));
      std::cout << "STOPPING CLIENT" << std::endl;
      client->stop_service();
      client.reset();
      std::cout << "STOPPED CLIENT" << std::endl;
    }
    if(renderer) {
      std::cout << "STOPPING RENDERER" << std::endl;
      renderer->stop_audiobackend();
      renderer.reset();
      std::cout << "STOPPED RENDERER" << std::endl;
    }
    if(mixer) {
      std::cout << "STOPPING MIXER" << std::endl;
      mixer->stop();
      std::cout << "STOPPED MIXER" << std::endl;
    }
    isRunning = false;
  }

private:
  bool shouldRun;
  bool isRunning;
  JackAudioController* jackAudioController;
  // std::unique_ptr<std::thread> killThread;
  std::unique_ptr<OvMixer> mixer;
  std::unique_ptr<ov_render_tascar_t> renderer;
  std::unique_ptr<ov_client_orlandoviols_t> client;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrlandoViolsClient)
};
