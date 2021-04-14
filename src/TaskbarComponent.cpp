#include "TaskbarComponent.h"
#include "../assets/utils.h"

TaskbarComponent::TaskbarComponent()
{
  setIconImage(getImageFromAssets("icon.png"),
               getImageFromAssets("icon@2x.png"));
  setIconTooltip("Digital Stage");
}

void TaskbarComponent::mouseDown(const MouseEvent&)
{
  Process::makeForegroundProcess();
  startTimer(50);
}

static void menuInvocationCallback(int chosenItemID, TaskbarComponent*)
{
  if(chosenItemID == 1)
    JUCEApplication::getInstance()->systemRequestedQuit();
}

void TaskbarComponent::timerCallback()
{
  stopTimer();

  PopupMenu m;
  m.addItem(1, "Quit");

  // It's always better to open menus asynchronously when possible.
  m.showMenuAsync(PopupMenu::Options(), ModalCallbackFunction::forComponent(
                                            menuInvocationCallback, this));
}