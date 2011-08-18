#ifndef PH_ERROR_H
#define PH_ERROR_H

#include <string>
#include <boost/format.hpp>

void warning(const std::string &str);
void warning(const boost::format &fmt);

void fatal(const std::string &str);
void fatal(const boost::format &fmt);

#endif /* PH_ERROR_H */
