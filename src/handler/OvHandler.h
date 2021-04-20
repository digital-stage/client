#pragma once
#include <Client.h>
#include <JuceHeader.h>
#include <Store.h>
#include <Types.h>
#include <mutex>
#include <optional>

#include "../../libov/src/ov_client_orlandoviols.h"
#include "../../libov/src/ov_render_tascar.h"

class OvHandler {
public:
  OvHandler(DigitalStage::Client* client_, const juce::File appDataDir);
  void init();

protected:
  void start();
  void stop();

  void syncLocalStageMember(const DigitalStage::Store& store);
  // void reduceStageMember();
  // void syncRemoteStageMembers();
  // void syncStageMemberPosition(const std::string& stageMemberId);
  // void syncStageMemberVolume(const std::string& stageMemberId);

  bool isRunning;
  DigitalStage::Client* client;
  std::unique_ptr<ov_render_tascar_t> renderer;

private:
  mutable std::recursive_mutex mutex;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OvHandler)
};
