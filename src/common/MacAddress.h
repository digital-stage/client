//
// Created by Tobias Hegemann on 06.05.21.
//

#ifndef MACADDRESS_H_
#define MACADDRESS_H_

#include <string>

class MacAddress {
public:
  static std::string getMacAddress() noexcept(false);
};

#endif // MACADDRESS_H_
