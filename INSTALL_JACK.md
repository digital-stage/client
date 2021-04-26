# Install and run JACK audio

All provided clients requires an installed and running JACK audio server.
To do this, please follow the guidelines for your operating system below:

## MacOS

### Install
Visit https://jackaudio.org/downloads/ and download the macOS Universal Installer.
Then open the downloaded DMG and run the jack-osx-*.pkg installer.
Also move the QjackCtl application into your applications folder, e.g. via Drag'n'Drop.

### Start
To start the Jack audio server, open the QjackCtl application inside your Applications folder and click 'Start', once the app opens up.

#### Configure
You may select your desired audio interface and set some settings like the sampling rate or buffer.
For this click on 'Settings' inside the QjackCtl application, assure that 'CoreAudio' is selected as driver, 'Realtime' is selected and set the audio interface, sample rate and buffer size to your desired values.
Good values are usually a sample rate of 48000Hz and a buffer size of 128.