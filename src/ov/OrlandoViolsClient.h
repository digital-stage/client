#pragma once
#include "Client.h"
#include "JackAudioController.h"
#include "OvMixer.h"
#include <JuceHeader.h>
#include <jack/jack.h>

#include "../../lib/libov/src/ov_client_orlandoviols.h"
#include "../../lib/libov/src/ov_render_tascar.h"

class OrlandoViolsClient {
public:
  OrlandoViolsClient(JackAudioController* controller,
                     const juce::String& appDataPath_)
      : appDataPath(appDataPath_), jackAudioController(controller)
  {
    mixer.reset(new OvMixer(appDataPath_));
    controller->addListener(
        [&](bool isAvailable) { handleJackAvailabilityChanged(isAvailable); });
  }

  inline void handleJackAvailabilityChanged(bool isAvailable)
  {
    if(isAvailable) {
      std::cout << "AVAILABLE" << std::endl;
      if(shouldRun && !isRunning) {
        std::cout << "START OV" << std::endl;
        startOv();
      }
    } else {
      std::cout << "UN AVAILABLE" << std::endl;
      if(isRunning) {
        std::cout << "STOP OV" << std::endl;
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
      std::cout << "ALREADY AVAILABLE" << std::endl;
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
    std::cout << "Init renderer" << std::endl;
    renderer.reset(new ov_render_tascar_t(getmacaddr(), 0));
    std::cout << "Init client" << std::endl;
    client.reset(new ov_client_orlandoviols_t(
        *renderer.get(), "http://oldbox.orlandoviols.com"));
    std::cout << "Set renderer and client" << std::endl;
    const juce::File zitaRootFolder =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::currentExecutableFile)
            .getParentDirectory();
    renderer->set_zita_path(zitaRootFolder.getFullPathName().toStdString() +
                            "/");
    renderer->set_runtime_folder(appDataPath.toStdString());
    client->set_runtime_folder(appDataPath.toStdString());
    std::cout << "Star service" << std::endl;
    client->start_service();
    isRunning = true;
  }

  inline void stopOv()
  {
    if(client) {
      std::cout << "STOPPING CLIENT" << std::endl;
      client->stop_service();
      client = nullptr;
    }
    if(renderer) {
      std::cout << "STOPPING RENDERER" << std::endl;
      renderer->stop_audiobackend();
      renderer = nullptr;
    }
    if(mixer) {
      std::cout << "STOPPING MIXER" << std::endl;
      mixer->stop();
    }
    isRunning = false;
  }

private:
  bool shouldRun;
  bool isRunning;
  const juce::String appDataPath;
  JackAudioController* jackAudioController;
  std::unique_ptr<OvMixer> mixer;
  std::unique_ptr<ov_render_tascar_t> renderer;
  std::unique_ptr<ov_client_orlandoviols_t> client;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrlandoViolsClient)
};
