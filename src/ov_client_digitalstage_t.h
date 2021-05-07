//
// Created by Tobias Hegemann on 28.04.21.
//

#ifndef OV_CLIENT_DIGITALSTAGE_T_H_
#define OV_CLIENT_DIGITALSTAGE_T_H_

#include "ov_ds_sockethandler_t.h"
#include <DigitalStage/Api/Client.h>
#include <nlohmann/json.hpp>
#include <ov_types.h>
#include <soundcardtools.h>
#include <utility>

using namespace DigitalStage::Api;
using namespace DigitalStage::Types;

class ov_client_digitalstage_t : public ov_client_base_t {
public:
  ov_client_digitalstage_t(ov_render_base_t& backend, std::string apiKey_)
      : ov_client_base_t(backend), apiKey(std::move(apiKey_)), shouldQuit(false)
  {
  }

  bool is_going_to_stop() const override { return shouldQuit; };

  inline void start_service() override
  {
    client = std::make_unique<DigitalStage::Api::Client>(API_URL);
    controller =
        std::make_unique<ov_ds_sockethandler_t>(&backend, client.get());
    controller->listen();

    client->ready.connect([&](const DigitalStage::Api::Store* store) {
      auto localDevice = store->getLocalDevice();

      // Get current

      auto tool = std::make_unique<sound_card_tools_t>();
      auto soundDevices = tool->get_sound_devices();
      for(const auto& soundDevice : soundDevices) {
        auto existingSoundCard = store->getSoundCardByUUID(soundDevice.id);
        if(existingSoundCard) {
          nlohmann::json payload;
          payload["softwareLatency"] = soundDevice.software_latency;
          payload["isDefault"] = soundDevice.is_default;
          payload["online"] = true;
          if(existingSoundCard->inputChannels.size() !=
             soundDevice.num_input_channels) {
            for(unsigned int i = 1; i <= soundDevice.num_input_channels; i++) {
              payload["outputDevices"].push_back(
                  {"system:capture_" + std::to_string(i), false});
            }
          }
          if(existingSoundCard->outputChannels.size() !=
             soundDevice.num_output_channels) {
            for(unsigned int i = 1; i <= soundDevice.num_output_channels; i++) {
              payload["outputDevices"].push_back(
                  {"system:playback_" + std::to_string(i), false});
            }
          }
          client->send(DigitalStage::Api::SendEvents::CHANGE_SOUND_CARD,
                       payload);
        } else {
          nlohmann::json payload;
          payload["uuid"] = soundDevice.id;
          payload["label"] = soundDevice.name;
          payload["driver"] = "jack";
          payload["drivers"] = {"jack"};
          payload["sampleRate"] = soundDevice.sample_rate;
          payload["sampleRates"] = soundDevice.sample_rates;
          payload["softwareLatency"] = soundDevice.software_latency;
          payload["isDefault"] = soundDevice.is_default;
          payload["numPeriods"] = 2;
          payload["online"] = true;
          for(unsigned int i = 1; i <= soundDevice.num_input_channels; i++) {
            payload["outputDevices"].push_back(
                {"system:capture_" + std::to_string(i), false});
          }
          for(unsigned int i = 1; i <= soundDevice.num_output_channels; i++) {
            payload["outputDevices"].push_back(
                {"system:playback_" + std::to_string(i), false});
          }
          if(!localDevice->soundCardId &&
             (soundDevice.is_default || soundDevices.size() == 1)) {
            // No sound card set yet, so use default
            client->send(DigitalStage::Api::SendEvents::SET_SOUND_CARD, payload,
                         [&, localDevice](const nlohmann::json& result) {
                           const std::string soundCardId = result[1];
                           nlohmann::json update = {
                               {"_id", localDevice->_id},
                               {"soundCardId", soundCardId}};
                           client->send("change-device", update);
                         });
          } else {
            client->send(DigitalStage::Api::SendEvents::SET_SOUND_CARD,
                         payload);
          }
        }
      }
    });

    // TODO: When local sound card is added: compare jack ports and update if
    // necessary
    // TODO: When local sound card is changed:

    // Send initial device
    nlohmann::json initialDevice;
    initialDevice["uuid"] = backend.get_deviceid();
    initialDevice["type"] = "ov";
    initialDevice["canAudio"] = true;
    initialDevice["canVideo"] = false;
    initialDevice["sendAudio"] = true;
    initialDevice["receiveAudio"] = true;
    client->connect(apiKey, initialDevice);
  }
  inline void stop_service() override
  {
    controller->unlisten();
    client->disconnect();
  }

private:
  const std::string apiKey;
  std::atomic<bool> shouldQuit;
  std::unique_ptr<DigitalStage::Api::Client> client;
  std::unique_ptr<ov_ds_sockethandler_t> controller;
};

#endif // OV_CLIENT_DIGITALSTAGE_T_H_
