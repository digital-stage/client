#include "OvMixer.h"
#include <chrono>
#include <iostream>

/**
 * We have to download the ov mixer, since the codesign tool of macOS does not
 * allow to contain any node based applications
 */
#ifdef JUCE_MAC
#define OV_MIXER_URL                                                           \
  "https://github.com/digital-stage/ov-webmixer/releases/download/Release/"    \
  "ov-webmixer-macos"
#elif JUCE_LINUX
#define OV_MIXER_URL                                                           \
  "https://github.com/digital-stage/ov-webmixer/releases/download/Release/"    \
  "ov-webmixer-linux"
#else
#define OV_MIXER_URL                                                           \
  "https://github.com/digital-stage/ov-webmixer/releases/download/Release/"    \
  "ov-webmixer-win.exe"
#endif

OvMixer::OvMixer()
    : webmixer(
          juce::File::getCurrentWorkingDirectory().getChildFile("webmixer")),
      isInitializing(false), quitRequested(false)
{
}

OvMixer::~OvMixer()
{
  stop();
}

void OvMixer::downloadAndStart()
{
  std::cout << "WEBMIXER: downloadAndStart()" << std::endl;
  // We don't wait for the download to finished
  if(!webmixer.existsAsFile()) {
    juce::URL* url = new juce::URL(OV_MIXER_URL);
    std::unique_ptr<juce::URL::DownloadTask> taskProgress =
        url->downloadToFile(webmixer);
    while(!quitRequested && taskProgress->isFinished() == false) {
      // Block
      std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    webmixer.setExecutePermission(true);
  }
  std::cout << "WEBMIXER: Creating child process" << std::endl;
  process.reset(new ChildProcess());
  if(!process->start(
         "./webmixer -r " +
         juce::File::getCurrentWorkingDirectory().getFullPathName())) {
    std::cout << "WEBMIXER: Could not start" << std::endl;
    process.reset();
  }
  isInitializing = false;
  std::cout << "WEBMIXER: RUNNING" << std::endl;
}

void OvMixer::start()
{
  if(!isInitializing && !process) {
    quitRequested = false;
    isInitializing = true;
    std::cout << "WEBMIXER: is NOT running, so starting it" << std::endl;
    downloadThread.reset(new std::thread(&OvMixer::downloadAndStart, this));
  }
}

void OvMixer::stop()
{
  std::cout << "WEBMIXER: stop()" << std::endl;
  quitRequested = true;
  if(process) {
    std::cout << "WEBMIXER: is running, killing it" << std::endl;
    process->kill();
  }
  process.reset();
  if(downloadThread && downloadThread->joinable()) {
    std::cout << "Waiting for download thread to finish" << std::endl;
    downloadThread->join();
    downloadThread.reset();
  }
}