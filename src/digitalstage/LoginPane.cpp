#include "LoginPane.h"
#include "../../assets/utils.h"
#include <thread>

void runAsync(std::function<void(void)> task,
              std::function<void(void)> completionHandler)
{
  std::thread th([task, completionHandler]() {
    task();
    MessageManager::callAsync(completionHandler);
  });
  th.detach();
  // the thread object being destroyed here doesn't matter, detach() has
  // persisted the thread
}

LoginPane::LoginPane()
{
  authService.reset(new DigitalStage::AuthService(AUTH_URL));
  logo.setImage(getImageFromAssets("digitalstage/logo-full@2x.png"));
  emailLabel.setText(TRANS("email"), dontSendNotification);
  passwordLabel.setText(TRANS("password"), dontSendNotification);
  loginButton.setButtonText(TRANS("login"));

  addAndMakeVisible(logo);
  addAndMakeVisible(errorLabel);
  addAndMakeVisible(emailLabel);
  addAndMakeVisible(emailEditor);
  addAndMakeVisible(passwordLabel);
  addAndMakeVisible(passwordEditor);
  addAndMakeVisible(loginButton);
  setSize(250, 400);

  loginButton.addListener(this);
}

LoginPane::~LoginPane()
{
  loginButton.removeListener(this);
  authService = nullptr;
}

void LoginPane::buttonClicked(juce::Button*)
{
  const juce::String email = emailEditor.getText();
  const juce::String password = passwordEditor.getText();
  if(email.length() > 0 && password.length() > 0) {
    std::thread th([&, email, password]() {
      const std::string result =
          authService->signInSync(email.toStdString(), password.toStdString());
      MessageManager::callAsync([&, result]() {
        if(result.length() > 0) {
          const juce::String token = juce::String(result);
          if(onSignedIn) {
            onSignedIn(token);
          }
        } else {
          errorLabel.setText(TRANS("invalidCredentials"), dontSendNotification);
          resized();
        }
      });
    });
    th.detach();
  }
}

void LoginPane::resized()
{
  juce::Grid grid;
  using Track = juce::Grid::TrackInfo;
  using Fr = juce::Grid::Fr;
  using Px = juce::Grid::Px;
  float rowMinHeight = 26.0f;
  grid.templateRows = {Track(Fr(2)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight)),
                       Track(Px(rowMinHeight))};
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
  if(errorLabel.getText().length() > 0) {
    grid.items.add(juce::GridItem(errorLabel)
                       .withMargin({0, verticalSpacing, 0, verticalSpacing}));
  }

  grid.performLayout(getLocalBounds());
}