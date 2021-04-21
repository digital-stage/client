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

OvMixer::OvMixer(const juce::File& appDataDir)
    : webmixer(appDataDir.getChildFile("webmixer")), isInitializing(false),
      quitRequested(false)
{
}

OvMixer::~OvMixer()
{
  stop();
}

void OvMixer::downloadAndStart()
{
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
  process.reset(new ChildProcess());
  if(!process->start("./webmixer")) {
    process = nullptr;
  }
  isInitializing = false;
}

void OvMixer::start()
{
  if(!isInitializing && !process) {
    quitRequested = false;
    isInitializing = true;
    downloadThread.reset(new std::thread(&OvMixer::downloadAndStart, this));
  }
}

void OvMixer::stop()
{
  quitRequested = true;
  if(process && process->isRunning()) {
    std::cout << "Killing webmixer process" << std::endl;
    process->kill();
    process = nullptr;
  }
  if(downloadThread && isInitializing) {
    std::cout << "Waiting for download thread to finish" << std::endl;
    downloadThread->join();
  }
}