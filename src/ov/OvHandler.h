#pragma once
#include "OvMixer.h"
#include <JuceHeader.h>
#include <ds/Client.h>
#include <ds/Store.h>
#include <ds/Types.h>
#include <mutex>
#include <optional>

#include "../../lib/libov/src/ov_client_orlandoviols.h"
#include "../../lib/libov/src/ov_render_tascar.h"

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
  std::unique_ptr<juce::AlertWindow> altert;
  std::unique_ptr<OvMixer> mixer;
  std::unique_ptr<ov_render_tascar_t> renderer;

private:
  bool isWaitingForJack;
  bool onOvStage;
  mutable std::recursive_mutex mutex;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OvHandler)
};
