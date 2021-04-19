#pragma once
#include "Client.h"
#include <JuceHeader.h>

#include "../libov/src/ov_client_orlandoviols.h"
#include "../libov/src/ov_render_tascar.h"

class OrlandoViolsClient {
public:
    OrlandoViolsClient() {}
    ~OrlandoViolsClient() {
      if( client ) {
        client->stop_service();
      }
    }
  inline void start() {
    //renderer->start_audiobackend();
    renderer.reset(new ov_render_tascar_t(getmacaddr(), 0));
    client.reset(new ov_client_orlandoviols_t(*renderer.get(),
                                                "http://oldbox.orlandoviols.com"));
    const std::string appFolder =
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getFullPathName()
            .toStdString();
    renderer->set_runtime_folder(appFolder);
    client->set_runtime_folder(appFolder);
    client->start_service();
  }
  inline void stop() {
    if( client ) {
      client->stop_service();
    }
  }

private:
std::unique_ptr<ov_render_tascar_t> renderer;
std::unique_ptr<ov_client_orlandoviols_t> client;
};
