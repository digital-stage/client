//
// Created by Tobias Hegemann on 27.04.21.
//

#include "OvHandler.h"
#include <DigitalStage/Types.h>
#include <eventpp/utilities/argumentadapter.h>
#include <iostream>
#include <memory>

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
      EventType::LOCAL_DEVICE_READY,
      eventpp::argumentAdapter(
          std::function<void(const EventLocalDeviceReady&, const Store&)>(
              [&](const EventLocalDeviceReady&, const Store&) {
                auto inputPorts = controller->getInputPorts();
                auto outputPorts = controller->getInputPorts();
                soundcard_t soundCard;
                soundCard.uuid = "default";
                soundCard.drivers.emplace_back("JACK");
                soundCard.driver = "JACK";
                soundCard.inputChannels = inputPorts;
                soundCard.inputChannels = outputPorts;

                std::cout << "TODO: SEND INPUTS" << std::endl;
                for(const std::string& el : inputPorts) {
                  std::cout << el << std::endl;
                }
                std::cout << "TODO: SEND OUTPUTS" << std::endl;
                for(const std::string& el : outputPorts) {
                  std::cout << el << std::endl;
                }
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
