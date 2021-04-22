#pragma once
#include "Client.h"
#include "JackAudioController.h"
#include "OvMixer.h"
#include <JuceHeader.h>
#include <jack/jack.h>

#include "../../lib/libov/src/ov_client_orlandoviols.h"
#include "../../lib/libov/src/ov_render_tascar.h"

#ifndef ORLANDOVIOLS_FRONTEND_URL
#define ORLANDOVIOLS_FRONTEND_URL "http://oldbox.orlandoviols.com"
#endif

class OrlandoViolsClient {
public:
  OrlandoViolsClient(JackAudioController* controller)
      : jackAudioController(controller)
  {
    mixer.reset(new OvMixer());
    controller->addListener(
        [&](bool isAvailable) { handleJackAvailabilityChanged(isAvailable); });
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

  inline void handleJackIsUnavailable() {}

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
    renderer.reset(new ov_render_tascar_t(getmacaddr(), 0));
    client.reset(new ov_client_orlandoviols_t(*renderer.get(),
                                              ORLANDOVIOLS_FRONTEND_URL));
    const std::string workingFolderPath =
        juce::File::getCurrentWorkingDirectory()
            .getFullPathName()
            .toStdString();
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
