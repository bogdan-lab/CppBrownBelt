#include "ini.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;


namespace Ini {




size_t Document::SectionCount() const {return doc_data.size();}

const Section& Document::GetSection(const string& sname) const {return doc_data.at(sname);}

Section& Document::AddSection(string sname){return doc_data[sname];}

Document Load(istream& input){
    Document doc;
    string line;
    Section* section = nullptr;
    while (getline(input, line)){
        if(!line.empty()){
            if(line.front()=='[' && line.back()==']'){
                section = &doc.AddSection(line.substr(1, line.size()-2));
            }
            else{
                size_t pos = line.find('=');
                section->insert({line.substr(0, pos), line.substr(pos + 1)});
            }
        }
    }
    return doc;
}

}

ostream& operator<<(ostream& out, const unordered_map<string, string>& sec){
    for(const auto& el : sec){
        out << el.first << " : " << el.second << '\n';
    }
    return out;
}
