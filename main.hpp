#ifndef INC_450PROJ3_MAIN_HPP
#define INC_450PROJ3_MAIN_HPP
#include <iostream>
#include <fstream>
#include <string>
// takes a binary string as an address and returns the corresponding physical address as a decimal
// translate virtual to physical
int translate_v2p(std::string bin_address);

// checks if the string given contains only numeric characters
bool isDecimal(std::string input);

// checks if the string given contains only numeric characters or A-F prefixed by 0x
bool isHex(std::string input);

// convert a decimal to a binary string
std::string dec2bin(int decimal);

// convert a binary string into a decimal integer
int bin2dec(std::string binary);

// convert a hex to a binary string, takes a hexadecimal string including "0x" prefix
std::string hex2bin(std::string hex);
// a helper for hex2bin
const char* hex2bin_helper(char c);

#endif //INC_450PROJ3_MAIN_HPP
