#pragma once

#include <JuceHeader.h>
#include <iostream>

inline File getRootDirectory()
{
  auto currentFile = File::getSpecialLocation(
      File::SpecialLocationType::currentApplicationFile);
  return currentFile.getParentDirectory();
}

inline std::unique_ptr<InputStream>
createAssetInputStream(const char* resourcePath)
{
#if JUCE_ANDROID
  ZipFile apkZip(File::getSpecialLocation(File::invokedExecutableFile));
  return std::unique_ptr<InputStream>(apkZip.createStreamForEntry(
      apkZip.getIndexOfFileName("assets/" + String(resourcePath))));
#else
#if JUCE_IOS
  auto assetsDir = File::getSpecialLocation(File::currentExecutableFile)
                       .getParentDirectory()
                       .getChildFile("assets");
#elif JUCE_MAC
  auto assetsDir = File::getSpecialLocation(File::currentExecutableFile)
                       .getParentDirectory()
                       .getParentDirectory()
                       .getChildFile("Resources")
                       .getChildFile("assets");

  if(!assetsDir.exists()) {
    assetsDir = getRootDirectory().getChildFile("assets");
  }
#else
  auto assetsDir = getRootDirectory().getChildFile("assets");
#endif

  auto resourceFile = assetsDir.getChildFile(resourcePath);

  jassert(resourceFile.existsAsFile());

  return resourceFile.createInputStream();
#endif
}

inline Image getImageFromAssets(const char* assetName)
{
  auto hashCode = (String(assetName) + "@assets").hashCode64();
  auto img = ImageCache::getFromHashCode(hashCode);

  if(img.isNull()) {
    std::unique_ptr<InputStream> juceIconStream(
        createAssetInputStream(assetName));

    if(juceIconStream == nullptr)
      return {};

    img = ImageFileFormat::loadFrom(*juceIconStream);

    ImageCache::addImageToCache(img, hashCode);
  }

  return img;
}