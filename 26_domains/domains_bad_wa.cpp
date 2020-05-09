#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <string_view>
#include <unordered_map>

using namespace std;

class DomainFilter{
private:
    unordered_map<string_view, vector<string_view>> data_;     //domain -> subdomains

    string_view GetMainDomain(const string_view address) const {
        size_t pos = address.find_last_of('.');
        if(pos==string::npos){return address;}
        else{return address.substr(pos+1, string::npos);}
    }

public:
    DomainFilter(const vector<string>& given_dommains){
        for(const auto& el : given_dommains){
            string_view main = GetMainDomain(el);
            data_[main].push_back(el);
        }
    }

    bool CheckDomainBelonging(string_view domain, string_view address) const {
        if(address.size()<domain.size()){return false;}
        if(address.size()==domain.size()){return domain==address;}
        size_t pos = address.size()-domain.size()-1;
        string_view check = address.substr(pos, string::npos);
        return check.front()=='.' && domain==check.substr(1, string::npos);
    }

    bool CheckForGood(const string_view domain) const {
        if(data_.empty()){return true;}
        string_view main = GetMainDomain(domain);
        if(data_.count(main)==0){return true;}
        for(const string_view el : data_.at(main)){
            if(CheckDomainBelonging(el, domain)){
                return false;
            }
        }
        return true;
    }

};

int main() {
    size_t count;
    cin >> count;
    vector<string> banned_domains;
    banned_domains.reserve(count);
    for(size_t i=0; i<count; i++){
        string tmp;
        cin >> tmp;
        banned_domains.push_back(move(tmp));
    }
    DomainFilter filter(banned_domains);
    cin >> count;
    for(size_t i=0; i<count; i++){
        string address;
        cin >> address;
        if (filter.CheckForGood(address)){
            cout << "Good\n";
        } else {
            cout << "Bad\n";
        }
    }
  return 0;
}
