#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include "test_runner.h"
using namespace std;


/*
bool IsSubdomain(string_view subdomain, string_view domain) {
  auto i = subdomain.size() - 1;
  auto j = domain.size() - 1;
  while (i >= 0 && j >= 0) {
    if (subdomain[i--] != domain[j--]) {
      return false;
    }
  }
  return (i < 0 && domain[j] == '.')
      || (j < 0 && subdomain[i] == '.');
}


vector<string> ReadDomains() {
  size_t count;
  cin >> count;

  vector<string> domains;
  for (size_t i = 0; i < count; ++i) {
    string domain;
    getline(cin, domain);
    domains.push_back(domain);
  }
  return domains;
}
*/


class DomainFilter{
private:
    set<string> banned_;
    size_t max_sub_dom_num_;

    void PopExternalDomain(string& given, string& sufix) const {
        size_t pos = given.rfind('.');
        if(pos==string::npos){
            sufix = given;
            given.clear();
        } else {
            sufix = given.substr(pos+1, string::npos);
            given.erase(pos);
        }
    }

    string ReverseNDomains(string& given, size_t count) const {
        string res;
        res.reserve(given.size());
        for(size_t i=0; i<count; i++){
            string sufix;
            PopExternalDomain(given, sufix);
            res+=sufix + '.';
            if(given.empty()){break;}
        }
        res.pop_back();
        return res;
    }

public:
    DomainFilter(vector<string> given_dommains): max_sub_dom_num_(0){
        for(auto& el : given_dommains){
            reverse(el.begin(), el.end());
            banned_.insert(el);
            //size_t domain_num = 1 + count(el.begin(), el.end(), '.');
            //if(domain_num>max_sub_dom_num_){max_sub_dom_num_ = domain_num;}
            //banned_.insert(ReverseNDomains(el, domain_num));
        }
    }

    set<string> GetBanned() const {return banned_; }
    size_t GetMaxSubNum() const {return max_sub_dom_num_;}

    bool CheckForGood(string& domain) const {
        if (banned_.empty()){return true;}
        string for_check = ReverseNDomains(domain, max_sub_dom_num_);
        auto range = banned_.equal_range(for_check);
        if(range.first == banned_.end()){
            return true;
        }
        else {
            for(auto it = range.first; it!=range.second; it++){
                if (*it==for_check.substr(0, it->size())){
                    return false;
                }
            }
        }
        return true;
    }
};



int main() {
    /*size_t count;
    cin >> count;
    vector<string> banned_domains(count ,string());
    for(size_t i=0; i<count; i++){
        cin >> banned_domains[i];
    }*/
    vector<string> banned_domains = {"ya.ru", "maps.me", "m.ya.ru", "com"};
    DomainFilter filter(move(banned_domains));
    vector<string> check = {"ya.ru", "ya.com", "m.maps.me", "moscow.m.ya.ru", "maps.com", "maps.ru", "ya.ya"};
    for(auto& el : check){
        cout << filter.CheckForGood(el);
    }
    /*cin >> count;
    for(size_t i=0; i<count; i++){
        string domain;
        cin >> domain;
        if(filter.CheckForGood(domain)){
            cout << "GOOD" << endl;
        } else {
            cout << "BAD" << endl;
        }
    }*/














    /*
  const vector<string> banned_domains = ReadDomains();
  const vector<string> domains_to_check = ReadDomains();

  for (string_view domain : banned_domains) {
    reverse(begin(domain), end(domain));
  }
  sort(begin(banned_domains), end(banned_domains));

  size_t insert_pos = 0;
  for (string& domain : banned_domains) {
    if (insert_pos == 0 || !IsSubdomain(domain, banned_domains[insert_pos - 1])) {
      swap(banned_domains[insert_pos++], domain);
    }
  }
  banned_domains.resize(insert_pos);

  for (const string_view domain : domains_to_check) {
    if (const auto it = upper_bound(begin(banned_domains), end(banned_domains), domain);
        it != begin(banned_domains) && IsSubdomain(domain, *prev(it))) {
      cout << "Good" << endl;
    } else {
      cout << "Bad" << endl;
    }
  }*/
  return 0;
}
