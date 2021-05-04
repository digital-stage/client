//
// Created by Tobias Hegemann on 27.04.21.
//
#include "JackAudioController.h"

#include <memory>

bool JackAudioController::isAvailable() const
{
  return isAvailable_;
}
void JackAudioController::setActive(bool active)
{
  if(active != isActive_) {
    isActive_ = active;
    if(isActive_) {
      // Start observer thread
      shouldExit = false;
      startObservation();
    } else {
      shouldExit = true;
      window->setVisible(false);
    }
  }
}
bool JackAudioController::isActive() const
{
  return isActive_;
}
void JackAudioController::addListener(
    const JackAudioController::ChangeListener& changeListener)
{
  changeListeners.push_back(changeListener);
}
void JackAudioController::removeAllListeners()
{
  changeListeners.clear();
}
JackAudioController::JackAudioController()
    : isActive_(false), isAvailable_(false), shouldExit(false)
{
  window = std::make_unique<JackNotAvailableWindow>();
}
JackAudioController::~JackAudioController()
{
  shouldExit = true;
  // Wait for observer thread to end
  if(observerThread && observerThread->joinable()) {
    observerThread->join();
  }
}
void JackAudioController::showDialog() {}
void JackAudioController::onAvailable()
{
  isAvailable_ = true;
  juce::MessageManager::callAsync([&]() { window->setVisible(false); });
  for(auto& c : changeListeners) {
    c(isAvailable_, serverSettings);
  }
}
void JackAudioController::onUnavailable()
{
  isAvailable_ = false;
  juce::MessageManager::callAsync([&]() {
    if(!shouldExit) {
      window->setVisible(true);
    }
  });
  for(auto& c : changeListeners) {
    c(isAvailable_, serverSettings);
  }
}
void JackAudioController::startObservation()
{
  if(!shouldExit) {
    if(observerThread && observerThread->joinable()) {
      observerThread->join();
    }
    observerThread =
        std::make_unique<std::thread>(&JackAudioController::observeJack, this);
  }
}
void JackAudioController::client_shutdown_cb(jack_status_t, const char*,
                                             void* args)
{
  auto instance = (JackAudioController*)args;
  instance->onUnavailable();
  // Restart observation again
  instance->startObservation();
}

void JackAudioController::scanJackAudioSizes(jack_client_t* jackClient)
{
  if(jackClient) {
    serverSettings.sampleRate = jack_get_sample_rate(jackClient);
    serverSettings.bufferSize = jack_get_buffer_size(jackClient);
  }
}
void JackAudioController::scanJackInputPorts(jack_client_t* jackClient)
{
  if(jackClient) {
    std::vector<std::string> ports;
    const char** pp_ports(jack_get_ports(
        jackClient, nullptr, nullptr, JackPortIsOutput | JackPortIsPhysical));
    if(pp_ports) {
      const char** p(pp_ports);
      while(*p) {
        ports.emplace_back(*p);
        ++p;
      }
      jack_free(pp_ports);
    }
    serverSettings.inputPorts = ports;
  }
}
void JackAudioController::scanJackOutputPorts(jack_client_t* jackClient)
{
  if(jackClient) {
    std::vector<std::string> ports;
    const char** pp_ports(jack_get_ports(jackClient, nullptr, nullptr,
                                         JackPortIsInput | JackPortIsPhysical));
    if(pp_ports) {
      const char** p(pp_ports);
      while(*p) {
        ports.emplace_back(*p);
        ++p;
      }
      jack_free(pp_ports);
    }
    serverSettings.outputPorts = ports;
  }
}
void JackAudioController::observeJack()
{
  jack_options_t options = JackNoStartServer;
  jack_status_t status;
  jack_client_t* jackClient =
      jack_client_open("observe_jack", options, &status);
  if(!jackClient) {
    onUnavailable();
  }
  while(!shouldExit && jackClient == nullptr) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    jackClient = jack_client_open("observe_jack", options, &status);
  }
  if( jackClient != nullptr ) {
    // Now fetch input and output ports
    scanJackInputPorts(jackClient);
    scanJackOutputPorts(jackClient);
    scanJackAudioSizes(jackClient);
    jack_on_info_shutdown(jackClient, client_shutdown_cb, (void*)this);
  }
  onAvailable();
}
std::vector<std::string> JackAudioController::getInputPorts() const
{
  return serverSettings.inputPorts;
}
std::vector<std::string> JackAudioController::getOutputPorts() const
{
  return serverSettings.outputPorts;
}
uint32_t JackAudioController::getBufferSize() const
{
  return serverSettings.bufferSize;
}
uint32_t JackAudioController::getSampleRate() const
{
  return serverSettings.sampleRate;
}
JackNotAvailableWindow::JackNotAvailableWindow()
    : juce::DocumentWindow(
          TRANS("JackUnvailable"),
          juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
              ResizableWindow::backgroundColourId),
          juce::DocumentWindow::TitleBarButtons::minimiseButton)
{
  label = std::make_unique<juce::Label>(
      "infoLabel",
      TRANS("Jack is not available. Please start it using QJackCtl."));
  label->setSize(200, 100);
  setContentOwned(label.get(), true);
}
