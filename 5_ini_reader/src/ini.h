#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace Ini {

using Section = unordered_map<string, string>;

//ostream& operator<<(ostream& out, const Section& sec);



class Document{
public:

    Section& AddSection(string name);
    const Section& GetSection(const string& name) const;
    size_t SectionCount() const;

private:
    unordered_map<string, Section> doc_data;
};

Document Load(istream& input);

vector<string> ParseOneLine(const string& line);
}
ostream& operator<<(ostream& out, const unordered_map<string, string>& sec);
