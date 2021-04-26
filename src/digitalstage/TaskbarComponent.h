#pragma once

#include "ApplicationState.h"
#include <JuceHeader.h>

class TaskbarComponent : public juce::SystemTrayIconComponent, private Timer {
public:
  TaskbarComponent(ApplicationState initalState);

  void setApplicationState(ApplicationState value);

  std::function<void()> onSignUpClicked;
  std::function<void()> onSignOutClicked;
  std::function<void()> onOpenStageClicked;
  std::function<void()> onOpenMixerClicked;
  std::function<void()> onSettingsClicked;

private:
  void buildPopup();
  void mouseDown(const MouseEvent& e) override;
  void timerCallback() override;

  ApplicationState state;
};
