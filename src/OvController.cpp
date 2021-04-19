#include "OvController.h"
#include "eventpp/utilities/argumentadapter.h"
#include <JuceHeader.h>

OvController::OvController(DigitalStage::Client* client_) : client(client_)
{
  auto mac = getmacaddr();
  thread.reset(new ServiceThread(mac));
}

void OvController::init()
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
                if(stage->audioType == "ov") {
                  std::cout << "HANDLING NOW OV FOR STAGE " << stage->name
                            << " AND GROUP " << group->name << std::endl;
                  start();
                }
              })));

  client->appendListener(
      DigitalStage::EventType::STAGE_LEFT,
      eventpp::argumentAdapter(
          std::function<void(const DigitalStage::EventStageLeft&,
                             const DigitalStage::Store&)>(
              [&](const DigitalStage::EventStageLeft&,
                  const DigitalStage::Store&) { stop(); })));
}

void OvController::start()
{
  if(!thread->isThreadRunning()) {
    thread->startThread(10);
  }
}

void OvController::stop()
{
  if(thread->isThreadRunning()) {
    thread->stopThread(1000);
  }
}

OvController::ServiceThread::ServiceThread(const std::string& uuid)
    : juce::Thread("ovcore")
{
  renderer.reset(new ov_render_tascar_t(uuid, 0));
  client.reset(new ov_client_orlandoviols_t(*renderer.get(),
                                            "http://oldbox.orlandoviols.com"));
  const std::string appFolder =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getFullPathName()
          .toStdString();
  renderer->set_runtime_folder(appFolder);
  client->set_runtime_folder(appFolder);
}

OvController::ServiceThread::~ServiceThread()
{
  stopThread(1000);
}

void OvController::ServiceThread::run()
{
  renderer->start_audiobackend();
  client->start_service();
  while(!threadShouldExit()) {
    wait(200);
  }
  client->stop_service();
}