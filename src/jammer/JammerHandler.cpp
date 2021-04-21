#include "JammerHandler.h"
#include "eventpp/utilities/argumentadapter.h"

JammerHandler::JammerHandler(DigitalStage::Client* client_) : client(client_) {}

void JammerHandler::init()
{
  client->appendListener(
      DigitalStage::EventType::STAGE_JOINED,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventStageJoined&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventStageJoined& e,
                  const DigitalStage::Store& s) {
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
      DigitalStage::EventType::STAGE_LEFT,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventStageLeft&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventStageLeft&,
                  const DigitalStage::Store&) {
                std::cout << "TODO: Shut down jammer client" << std::endl;
              })));
}