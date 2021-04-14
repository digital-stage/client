#pragma once

#include <JuceHeader.h>

class TaskbarComponent : public juce::SystemTrayIconComponent, private Timer {
public:
  TaskbarComponent();

  void mouseDown(const MouseEvent& e) override;

  void timerCallback() override;
};
