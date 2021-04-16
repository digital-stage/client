#include "LoginWindow.h"
#include "../assets/utils.h"

LoginWindow::LoginPane::LoginPane(int minWidth_, int minHeight_)
    : minWidth(minWidth_), minHeight(minHeight_)
{
  logo.setImage(getImageFromAssets("logo-full@2x.png"));
  emailLabel.setText(TRANS("email"), dontSendNotification);
  passwordLabel.setText(TRANS("password"), dontSendNotification);
  loginButton.setButtonText(TRANS("login"));

  addAndMakeVisible(logo);
  addAndMakeVisible(emailLabel);
  addAndMakeVisible(emailEditor);
  addAndMakeVisible(passwordLabel);
  addAndMakeVisible(passwordEditor);
  addAndMakeVisible(loginButton);

  loginButton.addListener(this);

  setSize(minWidth, minHeight);
}
LoginWindow::LoginPane::~LoginPane()
{
  loginButton.removeListener(this);
}
void LoginWindow::LoginPane::buttonClicked(juce::Button*)
{
  if(onSubmit) {
    const juce::String email = emailEditor.getText();
    const juce::String password = passwordEditor.getText();
    if(email.length() > 0 && password.length() > 0) {
      onSubmit(email, password);
    }
  }
}

void LoginWindow::LoginPane::resized()
{

  juce::Grid grid;

  using Track = juce::Grid::TrackInfo;
  using Fr = juce::Grid::Fr;
  using Px = juce::Grid::Px;

  // float rowMinWidth = minWidth;
  float rowMinHeight = minHeight / 7;
  grid.templateRows = {Track(Fr(2)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Fr(1))};
  grid.templateColumns = {Track(Fr(1))};

  float verticalSpacing = 20.0f;
  grid.items = {
      juce::GridItem(logo).withMargin({0, 0, 0, 0}),
      juce::GridItem(emailLabel)
          .withMargin({0, verticalSpacing, 0, verticalSpacing}),
      juce::GridItem(emailEditor)
          .withMargin({0, verticalSpacing, 0, verticalSpacing}),
      juce::GridItem(passwordLabel)
          .withMargin({0, verticalSpacing, 0, verticalSpacing}),
      juce::GridItem(passwordEditor)
          .withMargin({0, verticalSpacing, 0, verticalSpacing}),
      juce::GridItem(loginButton)
          .withMargin({0, verticalSpacing, 0, verticalSpacing}),
  };

  grid.performLayout(getLocalBounds());

  /*
   juce::FlexBox fb;
   fb.flexWrap = juce::FlexBox::Wrap::wrap;
   fb.flexDirection = juce::FlexBox::Direction::column;

   float rowMinWidth = minWidth;
   float rowMinHeight = minHeight / 6;
   fb.items.add(juce::FlexItem(logo)
                    .withFlex(1.0f, 0.f)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(250.0f));
   fb.items.add(juce::FlexItem(emailLabel)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(rowMinHeight));
   fb.items.add(juce::FlexItem(emailEditor)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(rowMinHeight));
   fb.items.add(juce::FlexItem(passwordLabel)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(rowMinHeight));
   fb.items.add(juce::FlexItem(passwordEditor)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(rowMinHeight));
   fb.items.add(juce::FlexItem(loginButton)
                    .withMinWidth(rowMinWidth)
                    .withMinHeight(rowMinHeight));

   fb.performLayout(getLocalBounds().toFloat());*/
}

LoginWindow::LoginWindow()
    : juce::DocumentWindow(
          TRANS("Login"),
          juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
              ResizableWindow::backgroundColourId),
          juce::DocumentWindow::TitleBarButtons::allButtons)
{
  setUsingNativeTitleBar(true);
#if JUCE_IOS || JUCE_ANDROID
  setFullScreen(true);
#else
  setResizable(true, true);
  centreWithSize(getWidth(), getHeight());
#endif

  int minWidth = 250;
  int minHeight = 300;
  pane.reset(new LoginPane(minWidth, minHeight));

  setContentOwned(pane.get(), true);

  pane->onSubmit = onSubmit;

  setResizeLimits(minWidth, minHeight, 600, 1200);
  setSize(minWidth, minHeight);
}

void LoginWindow::closeButtonPressed()
{
  // This is called when the user tries to close this window. Here, we'll just
  // ask the app to quit when this happens, but you can change this to do
  // whatever you need.
  JUCEApplication::getInstance()->systemRequestedQuit();
}
