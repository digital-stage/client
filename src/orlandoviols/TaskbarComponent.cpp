#include "TaskbarComponent.h"
#include "../../assets/utils.h"
#include <iostream>

enum MENU_IDENTIFIER {
  OPEN_STAGE = 1,
  OPEN_MIXER = 2,
  QUIT = 3
};

TaskbarComponent::TaskbarComponent()
{
  setIconImage(getImageFromAssets("orlandoviols/icon.png"),
               getImageFromAssets("orlandoviols/icon@2x.png"));
  setIconTooltip("Orlandoviols");
}

void TaskbarComponent::mouseDown(const MouseEvent&)
{
  Process::makeForegroundProcess();
  startTimer(50);
}

void TaskbarComponent::timerCallback()
{
  stopTimer();
  buildPopup();
}

static void menuInvocationCallback(int chosenItemID, TaskbarComponent* taskBar)
{
  switch(chosenItemID) {
  case MENU_IDENTIFIER::QUIT: {
    JUCEApplication::getInstance()->systemRequestedQuit();
    break;
  }
  case MENU_IDENTIFIER::OPEN_MIXER: {
    if(taskBar->onOpenMixerClicked) {
      taskBar->onOpenMixerClicked();
    }
    break;
  }
  case MENU_IDENTIFIER::OPEN_STAGE: {
    if(taskBar->onOpenStageClicked) {
      taskBar->onOpenStageClicked();
    }
    break;
  }
  }
}

void TaskbarComponent::buildPopup()
{
  PopupMenu m;

  m.addItem(MENU_IDENTIFIER::OPEN_STAGE, TRANS("Open stage"));
  m.addItem(MENU_IDENTIFIER::OPEN_MIXER, TRANS("Open mixer"));
  m.addItem(MENU_IDENTIFIER::QUIT, TRANS("Quit"));

  // It's always better to open menus asynchronously when possible.
  m.showMenuAsync(PopupMenu::Options(), ModalCallbackFunction::forComponent(
                                            menuInvocationCallback, this));
}
