#include "ov_client_digitalstage_t.h"
#include <DigitalStage/Auth/AuthService.h>

static bool quit_app(false);

static void sighandler(int)
{
  quit_app = true;
}

int main(int, char*[])
{
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);

  auto* authService = new DigitalStage::Auth::AuthService(AUTH_URL);
  auto jwt = authService->signInSync("tobias.hegemann@gmail.com",
                                     "vizqik-mawcIb-sohho3");

  ov_render_tascar_t renderer(getmacaddr(), 0);

  auto* client = new ov_client_digitalstage_t(renderer, jwt);

  std::cout << "starting services\n";
  client->start_service();

  while(!quit_app) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(client->is_going_to_stop()) {
      quit_app = true;
    }
  }

  std::cout << "stopping services\n";
  client->stop_service();
}