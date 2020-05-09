#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <string_view>
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
    size_t max_size_;
public:
    DomainFilter(vector<string> given_dommains):max_size_(0){
        for(auto el : given_dommains){
            reverse(el.begin(), el.end());
            banned_.insert(el);
            if(el.size()>max_size_){max_size_=el.size();}
        }
    }

    set<string> GetBanned() const {return banned_; }
    size_t GetMaxSubNum() const {return max_size_;}

    bool CompareReverseParts(const string& banned_dom, const string& cur_dom) const {
        if(banned_dom.size()==cur_dom.size()){
            return banned_dom==cur_dom;
        }
        bool res = banned_dom==cur_dom.substr(0, banned_dom.size());
        return res && cur_dom[banned_dom.size()]=='.';
    }

    bool CheckForGood(const string& domain) const {
        if (banned_.empty()){return true;}
        size_t pos;
        if(domain.size()<max_size_+1){pos = 0;}
        else{pos = domain.size() - max_size_-1;}
        string check = domain.substr(pos, string::npos);
        reverse(check.begin(), check.end());
        auto up_it = banned_.upper_bound(check);
        for(auto it = banned_.begin(); it!=up_it; it++){
            if (CompareReverseParts(*it, check)){
                return false;
            }
        }
        return true;
    }
};



int main() {
    /*
    vector<string> banned_domains = {"ya.ru", "maps.me", "m.ya.ru", "com"};
    DomainFilter filter(move(banned_domains));
    //vector<string> check = {"ya.ru", "ya.com", "m.maps.me", "moscow.m.ya.ru", "maps.com", "maps.ru", "ya.ya"};
    vector<string> check = {"ya.ya"};
    for(auto& el : check){
        cout << filter.CheckForGood(el);
    }*/

    size_t count;
    cin >> count;
    vector<string> banned_domains(count ,string());
    for(size_t i=0; i<count; i++){
        cin >> banned_domains[i];
    }
    DomainFilter filter(move(banned_domains));
    cin >> count;
    for(size_t i=0; i<count; i++){
        string domain;
        cin >> domain;
        if(filter.CheckForGood(domain)){
            cout << "Good\n";
        } else {
            cout << "Bad\n";
        }
    }

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
