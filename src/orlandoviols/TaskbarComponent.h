#pragma once

#include <JuceHeader.h>

class TaskbarComponent : public juce::SystemTrayIconComponent, private Timer {
public:
  TaskbarComponent();

  std::function<void()> onOpenStageClicked;
  std::function<void()> onOpenMixerClicked;

private:
  void buildPopup();
  void mouseDown(const MouseEvent& e) override;
  void timerCallback() override;
};
