#pragma once
#include <Client.h>
#include <JuceHeader.h>

#include "../libov/src/ov_client_orlandoviols.h"
#include "../libov/src/ov_render_tascar.h"

class OvHandler {
public:
  OvHandler(const DigitalStage::Client* client_);
  void init();

protected:
  void start();
  void stop();

private:
  class ServiceThread : public juce::Thread {
  public:
    ServiceThread(const std::string& uuid);
    ~ServiceThread();

    void run();

    std::unique_ptr<ov_render_tascar_t> renderer;
    std::unique_ptr<ov_client_orlandoviols_t> client;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ServiceThread)
  };

  DigitalStage::Client* client;
  std::unique_ptr<ServiceThread> thread;
};
