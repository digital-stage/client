#pragma once
#include <JuceHeader.h>
#include <iostream>
#include <jack/jack.h>
#include <jackclient.h>
#include <thread>

class JackAudioController {
public:
  JackAudioController();
  ~JackAudioController();

  void start();
  void stop();

  inline temp()
  {
    jack_options_t options = JackNoStartServer;
    jack_status_t status;
    jack_client_t* jackClient = jack_client_open("run_test", options, &status);
    if(jackClient != nullptr) {
      std::cout << "[ov_render_tascar] Jack is already running" << std::endl;
      jack_client_close(jackClient);
      return;
    }
    sprintf(cmd,
            "JACK_NO_AUDIO_RESERVATION=1 jackd --sync -P 40 -d coreaudio -d %s "
            "-r %g -p %d -n %d",
            devname.c_str(), audiodevice.srate, audiodevice.periodsize,
            audiodevice.numperiods);
    std::cout << "[ov_render_tascar] Starting jack server" << std::endl;
  }

  std::function<void()> onJackAvailable();
  std::function<void()> onJackUnAvailable();

private:
  bool isRunning;
  std::unique_ptr<std::thread> jackObserverThread;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JackAudioController)
};