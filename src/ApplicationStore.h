#pragma once
#include <JuceHeader.h>

class ApplicationStore : public juce::ApplicationProperties {
public:
  explicit ApplicationStore(const juce::String applicationName)
  {
    juce::PropertiesFile::Options options;
    options.applicationName = applicationName;
    options.doNotSave = false;
    options.commonToAllUsers = false;
    options.filenameSuffix = ".settings";
    options.ignoreCaseOfKeyNames = false;
    options.osxLibrarySubFolder = "Application Support/" + applicationName;
    setStorageParameters(options);
  }
};