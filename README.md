# native-client

Native client based on JUCE, api-client, libov and JammerNetz

## Running on macOS

### Install required software

Download and install JACK Audio from https://jackaudio.org/downloads/

### Download and install DigitalStage.app

#### Download

Download the latest release or nightly build from https://github.com/digital-stage/client/releases by collapsing the Assets panel and download the DMG.

#### Install

Open the DMG file and drag'n'drop the DigitalStage.app into your Applications folder.

![alt Download](https://github.com/digital-stage/client/blob/main/doc/download-release.png?raw=true)

#### Run

Since the experimental client is not signed properly by Apple yet, you need to start it the first time by pressing the option key and right click on it, then select "Open":

![alt Download](https://github.com/digital-stage/client/blob/main/doc/first-run.png?raw=true)

This will be only necessary on the first run after each installation. Later you can start it like any other application.

#### Starting the JACK Audio server

Our client needs access to the JACK Audio server.
For this open the qjackctl Application you've installed before and start the JACK server by clicking on "Start".
Ensure that the JACK server is running, otherwise you won't be able to use the client.

![alt Download](https://github.com/digital-stage/client/blob/main/doc/start-jack-audio.png?raw=true)

#### Switching to orlandoviols

We are currently combining live.digital-stage.org and the ov technology.
But since it is not available yet, you'll need to use the orlandoviols frontend.

To do so, click on the tray icon once you'v opened the app and select "Switch to orlandoviols..."

![alt Switch to orlandoviols.com](https://github.com/digital-stage/client/blob/main/doc/switch-to-orlandoviols.png?raw=true)

Now this client acts as a OVBox, but running on a mac :)

For more information how to use orlandoviols.com, please refer to
