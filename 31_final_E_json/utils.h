#ifndef UTILS_H
#define UTILS_H

#include <string_view>
#include <string>
#include <iostream>


namespace Utils {
std::string_view DeleteSpaces(std::string_view s);
std::string_view GetUntilSplitter(std::string_view& s, std::string_view splitter);
double ConvertToDouble(std::string_view s);
size_t ConvertToUInt(std::string_view s);
size_t ReadRequestCount(std::istream& in_stream);


}






#endif // UTILS_H
