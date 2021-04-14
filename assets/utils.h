#pragma once

#include <JuceHeader.h>

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
    if(currentFile.getFileName() == "examples")
      return currentFile;

    const auto sibling = currentFile.getSiblingFile("examples");

    if(sibling.exists())
      return sibling;

    currentFile = currentFile.getParentDirectory();
  }

  return currentFile;
#endif
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
                       .getChildFile("Assets");
#elif JUCE_MAC
  auto assetsDir = File::getSpecialLocation(File::currentExecutableFile)
                       .getParentDirectory()
                       .getParentDirectory()
                       .getChildFile("Resources")
                       .getChildFile("Assets");

  if(!assetsDir.exists())
    assetsDir = getExamplesDirectory().getChildFile("Assets");
#else
  auto assetsDir = getExamplesDirectory().getChildFile("Assets");
#endif

  auto resourceFile = assetsDir.getChildFile(resourcePath);
  jassert(resourceFile.existsAsFile());

  return resourceFile.createInputStream();
#endif
}

inline Image getImageFromAssets(const char* assetName)
{
  auto hashCode = (String(assetName) + "@juce_demo_assets").hashCode64();
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