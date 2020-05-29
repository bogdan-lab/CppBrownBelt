#include <string_view>
#include <string>
#include <iostream>


#include "utils.h"



namespace Utils {
std::string_view DeleteSpaces(std::string_view s){
    while(s.back()==' '){s.remove_suffix(1);}
    while(s.front()==' '){s.remove_prefix(1);}
    return s;
}

std::string_view GetUntilSplitter(std::string_view& s, std::string_view splitter){
    size_t pos = s.find(splitter.front());
    std::string_view info;
    if (pos != std::string::npos){
        info = s.substr(0, pos);
        s.remove_prefix(pos+splitter.size());
    }else {s.swap(info);}
    return DeleteSpaces(info);
}

double ConvertToDouble(std::string_view s){
    std::string_view tmp = DeleteSpaces(s);
    return std::stod(std::string(tmp), nullptr);
}


size_t ConvertToUInt(std::string_view s){
    std::string_view tmp = DeleteSpaces(s);
    return std::stoul(std::string(tmp), nullptr);
}

size_t ReadRequestCount(std::istream& in_stream){
    size_t request_count;
    in_stream >> request_count;
    std::string dummy;
    std::getline(in_stream, dummy);
    return request_count;
}


}


