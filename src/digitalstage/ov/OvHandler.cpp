//
// Created by Tobias Hegemann on 27.04.21.
//

#include "OvHandler.h"
#include <DigitalStage/Types.h>
#include <eventpp/utilities/argumentadapter.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using namespace DigitalStage;
using namespace DigitalStage::Types;

OvHandler::OvHandler(JackAudioController* controller_, Client* client_)
    : controller(controller_), client(client_)
{
  mixer = std::make_unique<OvMixer>();
}

void OvHandler::init()
{
  controller->addListener(
      [&](bool isAvailable,
          const JackAudioController::JackServerSettings& settings) {
        handleJackChanged(isAvailable, settings);
      });

  client->appendListener(
      EventType::SOUND_CARD_ADDED,
      eventpp::argumentAdapter(
          std::function<void(const EventSoundCardAdded&, const Store&)>(
              [&](const EventSoundCardAdded& e, const Store& s) {
                auto soundCardId = e.getSoundCard().uuid;
                if( soundCardId == "default" ) {
                  // The sound card
                  auto localDevice = s.getLocalDevice();
                  if(localDevice && localDevice->soundCardId != soundCardId) {
                    nlohmann::json update;
                    update["_id"] = localDevice->_id;
                    update["soundCardId"] = e.getSoundCard().uuid;
                    client->send("change-device", update.dump());
                  }
                  //TODO: Handle input and output channels
                }
              })));

  client->appendListener(
      EventType::SOUND_CARD_CHANGED,
      eventpp::argumentAdapter(
          std::function<void(const EventSoundCardChanged&, const Store&)>(
              [&](const EventSoundCardChanged& e, const Store& s) {
                auto soundCard = s.getSoundCard(e.getId());
                if( soundCard && soundCard->uuid == "default" ) {
                  //TODO: Handle input and output channels
                }
              })));

  client->appendListener(
      EventType::LOCAL_DEVICE_READY,
      eventpp::argumentAdapter(
          std::function<void(const EventLocalDeviceReady&, const Store&)>(
              [&](const EventLocalDeviceReady&, const Store&) {
                double sampleRate = controller->getSampleRate();
                soundcard_t soundCard;
                soundCard.uuid = "default";
                soundCard.label = "Jack";
                soundCard.drivers.emplace_back("JACK");
                soundCard.driver = "JACK";
                soundCard.inputChannels = controller->getInputPorts();
                soundCard.outputChannels = controller->getInputPorts();
                soundCard.isDefault = true;
                soundCard.periodSize = controller->getBufferSize();
                soundCard.numPeriods = 2;
                soundCard.sampleRate = sampleRate;
                soundCard.sampleRates = {sampleRate};
                client->send("set-sound-card", (nlohmann::json) soundCard);
              })));
}

void OvHandler::handleJackChanged(
    bool, const JackAudioController::JackServerSettings& settings)
{

  std::cout << "INPUT" << std::endl;
  for(const std::string& el : settings.inputPorts) {
    std::cout << el << std::endl;
  }
  std::cout << "OUTPUT" << std::endl;
  for(const std::string& el : settings.outputPorts) {
    std::cout << el << std::endl;
  }
}
OvHandler::~OvHandler() {}
