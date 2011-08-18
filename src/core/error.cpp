#include "precompiled.h"
#include "error.h"

void warning(const std::string &str)
{
    print("WARNING: ");
    print(str);
}

void warning(const format &fmt)
{
    warning(boost::str(fmt));
}

void fatal(const std::string &str)
{
    print("FATAL: ");
    print(str);

    if(!boost::ends_with(str, "\n"))
        print("\n");
    
    exit(-1);
}

void fatal(const format &fmt)
{
    fatal(boost::str(fmt));
}
