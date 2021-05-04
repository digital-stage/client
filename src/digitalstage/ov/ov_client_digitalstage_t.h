//
// Created by Tobias Hegemann on 28.04.21.
//

#ifndef OV_CLIENT_DIGITALSTAGE_T_H_
#define OV_CLIENT_DIGITALSTAGE_T_H_

#include "ov_controller_digitalstage_t.h"
#include <DigitalStage/Api/Client.h>
#include <nlohmann/json.hpp>
#include <ov_types.h>
#include <soundcardtools.h>
#include <utility>

using namespace DigitalStage::Api;

class ov_client_digitalstage_t : public ov_client_base_t {
public:
  ov_client_digitalstage_t(ov_render_base_t& backend, std::string apiKey_)
      : ov_render_base(backend_), apiKey(std::move(apiKey_))
  {
    // Create client
    client = std::make_unique<DigitalStage::Api::Client>(apiKey);
    controller = std::make_unique<ov_controller_digitalstage_t>(*client);

    client->ready([&](const Store& store) {
      // Send all available sound cards
      std::vector<snddevname_t> sound_cards = list_sound_devices();
      for(const auto& sound_card : sound_cards) {
        soundcard_t payload;
        payload.uuid = sound_card->dev;
        payload.label = sound_card->desc;
        payload.drivers.emplace_back("jack");
        payload.driver = "jack";
        // payload.inputChannels = jackAudioController->getInputPorts();
        // payload.outputChannels = jackAudioController->getInputPorts();
        payload.isDefault = true;
        // payload.periodSize = jackAudioController->getBufferSize();
        payload.numPeriods = 2;
        // payload.sampleRate = sampleRate;
        // payload.sampleRates = {sampleRate};
        client->sendAsync(DigitalStage::Api::SendEvents::SET_SOUND_CARD,
                          payload.dump());
      }
    });

    // Send initial device
    nlohmann::json initialDevice;
    initialDevice["uuid"] = backend.get_deviceid();
    initialDevice["type"] = "ov";
    initialDevice["canAudio"] = true;
    initialDevice["canVideo"] = false;
    client->connect(apiKey_, initialDevice);
  }

  inline void start_service() override { controller->start(); }
  inline void stop_service() override { controller->stop(); }

private:
  const std::string apiKey;
  std::unique_ptr<DigitalStage::Api::Client> client;
  std::unique_ptr<ov_controller_digitalstage_t> controller;
};

#endif // OV_CLIENT_DIGITALSTAGE_T_H_
