#pragma once

#include <JuceHeader.h>
#include <iostream>

inline File getExamplesDirectory() noexcept
{
#ifdef PIP_JUCE_EXAMPLES_DIRECTORY
  MemoryOutputStream mo;

  auto success = Base64::convertFromBase64(
      mo, JUCE_STRINGIFY(PIP_JUCE_EXAMPLES_DIRECTORY));
  ignoreUnused(success);
  jassert(success);

  return mo.toString();
#elif defined PIP_JUCE_EXAMPLES_DIRECTORY_STRING
  return File{CharPointer_UTF8{PIP_JUCE_EXAMPLES_DIRECTORY_STRING}};
#else
  auto currentFile = File::getSpecialLocation(
      File::SpecialLocationType::currentApplicationFile);
  auto exampleDir = currentFile.getParentDirectory().getChildFile("examples");

  if(exampleDir.exists())
    return exampleDir;

  // keep track of the number of parent directories so we don't go on endlessly
  for(int numTries = 0; numTries < 15; ++numTries) {
    if(currentFile.getFileName() == "assets")
      return currentFile;

    const auto sibling = currentFile.getSiblingFile("assets");

    if(sibling.exists())
      return sibling;

    currentFile = currentFile.getParentDirectory();
  }

  return currentFile;
#endif
}

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