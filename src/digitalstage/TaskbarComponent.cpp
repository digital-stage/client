#include "TaskbarComponent.h"
#include "../../assets/utils.h"
#include <iostream>

enum MENU_IDENTIFIER {
  SIGN_OUT = 1,
  SIGN_UP = 2,
  OPEN_STAGE = 3,
  OPEN_MIXER = 4,
  SETTINGS = 5,
  QUIT = 6
};

TaskbarComponent::TaskbarComponent(ApplicationState initialState)
    : state(initialState)
{
  setIconImage(getImageFromAssets("digitalstage/icon.png"),
               getImageFromAssets("digitalstage/icon@2x.png"));
  setIconTooltip("Digital Stage");
}

void TaskbarComponent::mouseDown(const MouseEvent&)
{
  Process::makeForegroundProcess();
  startTimer(50);
}

void TaskbarComponent::setApplicationState(ApplicationState value)
{
  state = value;
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
  case MENU_IDENTIFIER::SIGN_UP: {
    if(taskBar->onSignUpClicked)
      taskBar->onSignUpClicked();
    break;
  }
  case MENU_IDENTIFIER::SIGN_OUT: {
    if(taskBar->onSignOutClicked) {
      taskBar->onSignOutClicked();
    }
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
  case MENU_IDENTIFIER::SETTINGS: {
    if(taskBar->onSettingsClicked) {
      taskBar->onSettingsClicked();
    }
    break;
  }
  }
}

void TaskbarComponent::buildPopup()
{
  PopupMenu m;

  switch(state) {
  case OUTSIDE_STAGE: {
    m.addItem(MENU_IDENTIFIER::SETTINGS, TRANS("Settings"));
    m.addItem(MENU_IDENTIFIER::SIGN_OUT, TRANS("Sign out"));
    break;
  }
  case OV: {
    m.addItem(MENU_IDENTIFIER::OPEN_STAGE, TRANS("Open stage"));
    m.addItem(MENU_IDENTIFIER::OPEN_MIXER, TRANS("Open mixer"));
    m.addItem(MENU_IDENTIFIER::SETTINGS, TRANS("Settings"));
    m.addItem(MENU_IDENTIFIER::SIGN_OUT, TRANS("Sign out"));
    break;
  }
  case JAMMER: {
    m.addItem(MENU_IDENTIFIER::OPEN_STAGE, TRANS("Open stage"));
    m.addItem(MENU_IDENTIFIER::OPEN_MIXER, TRANS("Open mixer"));
    m.addItem(MENU_IDENTIFIER::SETTINGS, TRANS("Settings"));
    m.addItem(MENU_IDENTIFIER::SIGN_OUT, TRANS("Sign out"));
    break;
  }
  case SIGNED_OUT: {
    m.addItem(MENU_IDENTIFIER::SIGN_UP, TRANS("Sign up"));
    m.addItem(MENU_IDENTIFIER::SETTINGS, TRANS("Settings"));
    break;
  }
  }
  m.addItem(MENU_IDENTIFIER::QUIT, TRANS("Quit"));

  // It's always better to open menus asynchronously when possible.
  m.showMenuAsync(PopupMenu::Options(), ModalCallbackFunction::forComponent(
                                            menuInvocationCallback, this));
}
