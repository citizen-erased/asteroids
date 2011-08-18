#ifndef PH_LOGGER_H
#define PH_LOGGER_H

#include <boost/format.hpp>
#include <vector>
#include <string>
#include <ostream>

using boost::format; //HACK everything can use boost::format!

void print(const format &fmt);
void print(const std::string &str);

#endif /* PH_LOGGER_H */

