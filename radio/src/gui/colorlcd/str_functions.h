#pragma once
#include <wctype.h>
#include <string>
#include <iostream>
#include <sstream>

extern char *trim(char *str);
extern std::string trim_start(std::string str);
extern std::string wrap(std::string str, uint32_t width);



