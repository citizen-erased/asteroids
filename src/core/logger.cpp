#include "precompiled.h"
#include <iostream>
#include "logger.h"

void print(const format &fmt)
{
    print(boost::str(fmt));
}

void print(const std::string &str)
{
    std::cout << str;

#ifdef WIN32
    OutputDebugStringA(str.c_str());
#endif
}