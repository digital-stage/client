#include "JammerHandler.h"

using namespace DigitalStage::Api;

JammerHandler::JammerHandler(Client* client_) : client(client_) {}

void JammerHandler::init()
{
  client->stageJoined.connect(
      [&](const ID_TYPE& stageId, const ID_TYPE& groupId, const Store* store) {
        auto stage = store->getStage(stageId);
        auto group = store->getGroup(groupId);
        if(stage->audioType == "jammer") {
          std::cout << "TODO: Start jammer session for stage " << stage->name
                    << " and group " << group->name << " using ipv4 "
                    << *stage->jammerIpv4 << " and port " << *stage->jammerPort
                    << std::endl;
        }
      });
  client->stageLeft.connect([&](const Store*) {
    std::cout << "TODO: Shut down jammer client" << std::endl;
  });
}