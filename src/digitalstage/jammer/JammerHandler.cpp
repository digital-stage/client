#include "JammerHandler.h"
#include <eventpp/utilities/argumentadapter.h>

using namespace DigitalStage::Api;

JammerHandler::JammerHandler(Client* client_) : client(client_) {}

void JammerHandler::init()
{
  client->appendListener(
      EventType::STAGE_JOINED,
      eventpp::argumentAdapter(
          std::function<void(const EventStageJoined&,
                             const Store&)>(
              [&](const EventStageJoined& e,
                  const Store& s) {
                auto stage = s.getStage(e.getStageId());
                auto group = s.getGroup(e.getGroupId());
                if(stage->audioType == "jammer") {
                  std::cout << "TODO: Start jammer session for stage "
                            << stage->name << " and group " << group->name
                            << " using ipv4 " << *stage->jammerIpv4
                            << " and port " << *stage->jammerPort << std::endl;
                }
              })));

  client->appendListener(
      EventType::STAGE_LEFT,
      eventpp::argumentAdapter(
          std::function<void(const EventStageLeft&,
                             const Store&)>(
              [&](const EventStageLeft&,
                  const Store&) {
                std::cout << "TODO: Shut down jammer client" << std::endl;
              })));
}