//
// Created by Tobias Hegemann on 27.04.21.
//

#include "OvHandler.h"
#include "../../ov_ds_sockethandler_t.h"
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
}

void OvHandler::init()
{
  // JACK AUDIO SERVER LISTENER
  jackAudioController->addListener(
      [&](bool isAvailable,
          const JackAudioController::JackServerSettings& settings) {
        handleJackChanged(isAvailable, settings);
      });

  client->ready.connect(&OvHandler::handleReady, this);
}

void OvHandler::handleReady(const Store* store)
{
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

  auto soundCard = store->getSoundCardByUUID("manual");
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

  auto localDevice = store->getLocalDevice();
  if(!localDevice) {
    throw std::runtime_error("Internal error: no local device available");
  }
  client->send("set-sound-card", payload,
               [&, localDevice](const nlohmann::json& result) {
                 // Expecting (error: string | null, id: string)
                 // Step 2
                 // Assure that sound card is selected
                 const std::string soundCardId = result[1];
                 nlohmann::json update = {
                     {"_id", localDevice->_id},
                     {"soundCardId", soundCardId},
                     {"availableSoundCardIds", {soundCardId}}};
                 client->send("change-device", update);
               });
}

void OvHandler::handleJackChanged(
    bool isAvailable, const JackAudioController::JackServerSettings&)
{
  if(isAvailable) {
    if(!isRunning) {
      std::cout << "STARTING OV" << std::endl;
      mixer->start();
      // controller->start();
      isRunning = true;
    }
  } else if(isRunning) {
    std::cout << "STOPPING OV" << std::endl;
    mixer->stop();
    // controller->stop();
    isRunning = false;
  }
}