//
// Created by Tobias Hegemann on 27.04.21.
//

#include "OvHandler.h"
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using namespace DigitalStage;
using namespace DigitalStage::Types;

bool comparePorts(std::map<std::string, bool>& lhs,
                  std::map<std::string, bool>& rhs)
{
  if(lhs.size() != rhs.size())
    return false;
  for(auto& item : lhs) {
    if(rhs.count(item.first) == 0)
      return false;
  }
  return true;
}

OvHandler::OvHandler(JackAudioController* controller_, Client* client_)
    : jackAudioController(controller_), client(client_), isRunning(false)
{
  mixer = std::make_unique<OvMixer>();
  renderer = std::make_unique<ov_render_tascar_t>(getmacaddr(), 0);
  const std::string workingFolderPath =
      juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString() +
      "/";
  const juce::File zitaRootFolder =
      juce::File::getSpecialLocation(
          juce::File::SpecialLocationType::currentExecutableFile)
          .getParentDirectory();
  renderer->set_zita_path(zitaRootFolder.getFullPathName().toStdString() + "/");
  renderer->set_runtime_folder(workingFolderPath);
  controller = std::make_unique<ov_ds_sockethandler_t>(renderer.get(), client);
  jackAudioController->addListener(
      [&](bool isAvailable,
          const JackAudioController::JackServerSettings& settings) {
        handleJackChanged(isAvailable, settings);
      });

  client->ready.connect(&OvHandler::onReady, this);
}

OvHandler::~OvHandler()
{
  controller->disable();
}

void OvHandler::onReady(const Store*)
{
  if(jackAudioController->isAvailable()) {
    start();
  }
}

void OvHandler::handleJackChanged(
    bool isAvailable, const JackAudioController::JackServerSettings&)
{
  std::cout << "OvHandler::handleJackChanged" << std::endl;
  if(isAvailable) {
    start();
  } else {
    stop();
  }
}

void OvHandler::sendSoundCard()
{
  auto localDevice = client->getStore()->getLocalDevice();
  if(!localDevice) {
    throw std::runtime_error("Internal error: no local device available");
  }
  double sampleRate = jackAudioController->getSampleRate();
  double bufferSize = jackAudioController->getBufferSize();
  std::map<std::string, bool> inputPorts;
  auto inputPortNames = jackAudioController->getInputPorts();
  for(std::size_t i = 0; i < inputPortNames.size(); ++i) {
    inputPorts[inputPortNames.at(i)] = i < 2;
  }
  std::map<std::string, bool> outputPorts;
  auto outputPortNames = jackAudioController->getOutputPorts();
  for(std::size_t i = 0; i < outputPortNames.size(); ++i) {
    outputPorts[outputPortNames.at(i)] = i < 2;
  }
  nlohmann::json payload;
  payload["uuid"] = "manual";
  payload["label"] = "Jack";
  payload["drivers"] = {"jack"};
  payload["driver"] = "jack";
  payload["isDefault"] = true;
  payload["numPeriods"] = 2;
  payload["sampleRate"] = sampleRate;
  payload["sampleRates"] = {sampleRate};
  payload["periodSize"] = bufferSize;

  auto soundCard = client->getStore()->getSoundCardByDeviceAndUUID(
      localDevice->_id, "manual");
  if(soundCard) {
    if(!comparePorts(soundCard->inputChannels, inputPorts)) {
      payload["inputChannels"] = inputPorts;
    }
    if(!comparePorts(soundCard->outputChannels, outputPorts)) {
      payload["outputChannels"] = outputPorts;
    }
  } else {
    payload["inputChannels"] = inputPorts;
    payload["outputChannels"] = outputPorts;
  }
  client->send(DigitalStage::Api::SendEvents::SET_SOUND_CARD, payload,
               [&, localDevice](const nlohmann::json& result) {
                 // Expecting (error: string | null, id: string)
                 // Assure that sound card is selected
                 const std::string soundCardId = result[1];
                 nlohmann::json update = {{"_id", localDevice->_id},
                                          {"soundCardId", soundCardId}};
                 client->send(DigitalStage::Api::SendEvents::CHANGE_DEVICE,
                              update);
               });
}

void OvHandler::start()
{
  if(!isRunning && client->getStore()->isReady()) {
    std::cout << "STARTING OV" << std::endl;
    sendSoundCard();
    controller->enable();
    mixer->start();
    isRunning = true;
  }
}

void OvHandler::stop()
{
  if(!isRunning) {
    std::cout << "STOPPING OV" << std::endl;
    mixer->stop();
    controller->disable();
    isRunning = false;
  }
}
