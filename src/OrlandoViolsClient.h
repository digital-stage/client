#pragma once
#include "Client.h"
#include <JuceHeader.h>

#include "../libov/src/ov_client_orlandoviols.h"
#include "../libov/src/ov_render_tascar.h"

class OrlandoViolsClient {
public:
  OrlandoViolsClient(const std::string& appDataPath_)
      : appDataPath(appDataPath_)
  {
  }
  ~OrlandoViolsClient()
  {
    if(client) {
      client->stop_service();
    }
  }
  inline void start()
  {
    // renderer->start_audiobackend();
    renderer.reset(new ov_render_tascar_t(getmacaddr(), 0));
    client.reset(new ov_client_orlandoviols_t(
        *renderer.get(), "http://oldbox.orlandoviols.com"));
    renderer->set_runtime_folder(appDataPath);
    client->set_runtime_folder(appDataPath);
    client->start_service();
  }
  inline void stop()
  {
    if(client) {
      client->stop_service();
    }
  }

private:
  const std::string appDataPath;
  std::unique_ptr<ov_render_tascar_t> renderer;
  std::unique_ptr<ov_client_orlandoviols_t> client;
};
