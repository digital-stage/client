//
// Created by Tobias Hegemann on 06.05.21.
//

#include "MacAddress.h"

#include "juce_core/juce_core.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>

static kern_return_t FindEthernetInterfaces(io_iterator_t* matchingServices)
{
  kern_return_t kernResult;
  CFMutableDictionaryRef matchingDict;
  CFMutableDictionaryRef propertyMatchDict;

  matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

  if(nullptr != matchingDict) {
    propertyMatchDict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    if(nullptr != propertyMatchDict) {
      CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface),
                           kCFBooleanTrue);
      CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey),
                           propertyMatchDict);
      CFRelease(propertyMatchDict);
    }
  }
  kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict,
                                            matchingServices);
  return kernResult;
}

static kern_return_t GetMACAddress(io_iterator_t intfIterator,
                                   UInt8* MACAddress, UInt8 bufferSize)
{
  io_object_t intfService;
  io_object_t controllerService;
  kern_return_t kernResult = KERN_FAILURE;

  if(bufferSize < kIOEthernetAddressSize) {
    return kernResult;
  }

  bzero(MACAddress, bufferSize);

  while((intfService = IOIteratorNext(intfIterator))) {
    CFTypeRef MACAddressAsCFData;
    kernResult = IORegistryEntryGetParentEntry(intfService, kIOServicePlane,
                                               &controllerService);
    if(KERN_SUCCESS != kernResult) {
      printf("IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
    } else {
      // Retrieve the MAC address property from the I/O Registry in the form of
      // a CFData
      MACAddressAsCFData = IORegistryEntryCreateCFProperty(
          controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
      if(MACAddressAsCFData) {
        // Get the raw bytes of the MAC address from the CFData
        CFDataGetBytes((CFDataRef)MACAddressAsCFData,
                       CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
        CFRelease(MACAddressAsCFData);
      }
      // Done with the parent Ethernet controller object so we release it.
      (void)IOObjectRelease(controllerService);
    }

    // Done with the Ethernet interface object so we release it.
    (void)IOObjectRelease(intfService);
  }
  return kernResult;
}

static long GetMACAddressMAC(unsigned char* result)
{
  io_iterator_t intfIterator;
  kern_return_t kernResult = KERN_FAILURE;
  do {
    kernResult = ::FindEthernetInterfaces(&intfIterator);
    if(KERN_SUCCESS != kernResult)
      break;
    kernResult = ::GetMACAddress(intfIterator, (UInt8*)result, 6);
  } while(false);
  (void)IOObjectRelease(intfIterator);
  return kernResult;
}
#endif

std::string MacAddress::getMacAddress() noexcept(false) {
#ifdef __APPLE__
  unsigned char mac_address[6];
  if(GetMACAddressMAC(mac_address) == 0) {
    char ctmp[1024];
    std::sprintf(ctmp, "%02x%02x%02x%02x%02x%02x", mac_address[0], mac_address[1],
            mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
    return ctmp;
  }
#endif
  auto addresses = juce::MACAddress::getAllAddresses();
  if( !addresses.isEmpty() ) {
    return addresses[0].toString("").toStdString();
  }
  throw std::runtime_error("Could not obtain any MAC address");
}