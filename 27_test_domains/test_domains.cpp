#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

template <typename It>
class Range {
public:
  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

vector<string_view> Split(string_view s, string_view delimiter = " ") {
  vector<string_view> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.push_back(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}


class Domain {
public:
  explicit Domain(string_view text) {
    vector<string_view> parts = Split(text, ".");
    parts_reversed_.assign(rbegin(parts), rend(parts));
  }

  size_t GetPartCount() const {
    return parts_reversed_.size();
  }

  auto GetParts() const {
    return Range(rbegin(parts_reversed_), rend(parts_reversed_));
  }
  auto GetReversedParts() const {
    return Range(begin(parts_reversed_), end(parts_reversed_));
  }

  bool operator==(const Domain& other) const {
    return parts_reversed_ == other.parts_reversed_;
  }

private:
  vector<string> parts_reversed_;
};

ostream& operator<<(ostream& stream, const Domain& domain) {
  bool first = true;
  for (const string_view part : domain.GetParts()) {
    if (!first) {
      stream << '.';
    } else {
      first = false;
    }
    stream << part;
  }
  return stream;
}

// domain is subdomain of itself
bool IsSubdomain(const Domain& subdomain, const Domain& domain) {
  const auto subdomain_reversed_parts = subdomain.GetReversedParts();
  const auto domain_reversed_parts = domain.GetReversedParts();
  return
      subdomain.GetPartCount() >= domain.GetPartCount()
      && equal(begin(domain_reversed_parts), end(domain_reversed_parts),
               begin(subdomain_reversed_parts));
}

bool IsSubOrSuperDomain(const Domain& lhs, const Domain& rhs) {
  return lhs.GetPartCount() >= rhs.GetPartCount()
         ? IsSubdomain(lhs, rhs)
         : IsSubdomain(rhs, lhs);
}


class DomainChecker {
public:
  template <typename InputIt>
  DomainChecker(InputIt domains_begin, InputIt domains_end) {
    sorted_domains_.reserve(distance(domains_begin, domains_end));
    for (const Domain& domain : Range(domains_begin, domains_end)) {
      sorted_domains_.push_back(&domain);
    }
    sort(begin(sorted_domains_), end(sorted_domains_), IsDomainLess);
    sorted_domains_ = AbsorbSubdomains(move(sorted_domains_));
  }

  // Check if candidate is subdomain of some domain
  bool IsSubdomain(const Domain& candidate) const {
    const auto it = upper_bound(
        begin(sorted_domains_), end(sorted_domains_),
        &candidate, IsDomainLess);
    if (it == begin(sorted_domains_)) {
      return false;
    }
    return ::IsSubdomain(candidate, **prev(it));
  }

private:
  vector<const Domain*> sorted_domains_;

  static bool IsDomainLess(const Domain* lhs, const Domain* rhs) {
    const auto lhs_reversed_parts = lhs->GetReversedParts();
    const auto rhs_reversed_parts = rhs->GetReversedParts();
    return lexicographical_compare(
      begin(lhs_reversed_parts), end(lhs_reversed_parts),
      begin(rhs_reversed_parts), end(rhs_reversed_parts)
    );
  }

  static vector<const Domain*> AbsorbSubdomains(vector<const Domain*> domains) {
    domains.erase(
        unique(begin(domains), end(domains),
               [](const Domain* lhs, const Domain* rhs) {
                 return IsSubOrSuperDomain(*lhs, *rhs);
               }),
        end(domains)
    );
    return domains;
  }
};


vector<Domain> ReadDomains(istream& in_stream = cin) {
  vector<Domain> domains;

  size_t count;
  in_stream >> count;
  domains.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    string domain_text;
    in_stream >> domain_text;
    domains.emplace_back(domain_text);
  }
  return domains;
}

vector<bool> CheckDomains(const vector<Domain>& banned_domains, const vector<Domain>& domains_to_check) {
  const DomainChecker checker(begin(banned_domains), end(banned_domains));

  vector<bool> check_results;
  check_results.reserve(domains_to_check.size());
  for (const Domain& domain_to_check : domains_to_check) {
    check_results.push_back(!checker.IsSubdomain(domain_to_check));
  }

  return check_results;
}

void PrintCheckResults(const vector<bool>& check_results, ostream& out_stream = cout) {
  for (const bool check_result : check_results) {
    out_stream << (check_result ? "Good" : "Bad") << "\n";
  }
}

void TestSplitting(){
    string tst = "this.is.test.string";
    vector<string_view> exprected = {"this","is","test","string"};
    vector<string_view> res = Split(tst, ".");
    ASSERT_EQUAL(res, exprected);
    /*{
        string s = "a b cd";
        auto res = Split(s, " ");
        vector<string_view> v{ "a", "b", "cd" };
        ASSERT_EQUAL(res, v);
    }*/
}


void TestReversedPart(){
    Domain dom("this.is.test.string");
    vector<string> expected = {"string", "test", "is", "this"};
    vector<string> res;
    for(auto it : dom.GetReversedParts()){
        res.push_back(it);
    }
    ASSERT_EQUAL(res, expected);
    /*{
        Domain d = Domain("mail.ya.ru");
        auto res = d.GetReversedParts();
        vector<string> resv = { res.begin(), res.end() };
        auto expected = vector<string>{ "ru", "ya", "mail" };
        ASSERT_EQUAL(resv, expected);
    }*/
}

void TestSubdomain(){
    Domain dom("com");
    Domain sub("sub.com");
    ASSERT(IsSubdomain(sub, dom));
    ASSERT(IsSubdomain(dom, dom));
    ASSERT(!IsSubdomain(dom, sub));
    //not mine... checked wrong function...
    vector<Domain> v = { Domain("mail.ya.ru") };
    DomainChecker ch(v.begin(), v.end());
    ASSERT(ch.IsSubdomain(v[0]));
    Domain d2 = Domain("ya.ru");
    ASSERT(!ch.IsSubdomain(d2));
    Domain d3 = Domain("m2.mail.ya.ru");
    ASSERT(ch.IsSubdomain(d3));
}

void TestAbsorption(){
    Domain d1("google.com");
    Domain d2("maps.google.com");
    vector<Domain> tst = {d1, d2};
    DomainChecker dc(tst.begin(), tst.end());
    Domain t_dom("mail.google.com");
    ASSERT(dc.IsSubdomain(t_dom));
    //not mine
    vector<Domain> banned = { Domain("ya.ru"), Domain("mail.ya.ru"), Domain("mail.ru") };
    vector<Domain> to_check = {
        Domain("ya.ru"), Domain("mail.ya.ru"), Domain("mail.ru"),
        Domain("ru"), Domain(""), Domain("ba.ya.ru"), Domain("a.a.a.a.ya.ru")
    };
    auto res = CheckDomains(banned, to_check);
    vector<bool> exp = { false, false, false, true, true, false, false };
    ASSERT_EQUAL(res, exp);
}

void TestBadSubdomain(){
    vector<Domain> banned = {Domain("com"), Domain("ya.ru")};
    vector<Domain> check = {Domain("bad.com"), Domain("good.ru")};
    vector<bool> check_vals = CheckDomains(banned, check);
    ASSERT(!check_vals[0]);
    ASSERT(check_vals[1]);
    /*{
            vector<Domain> banned = { Domain("ya.ru"), Domain("1.ya.ru"), Domain("3.ya.ru") };
            vector<Domain> to_check = {
                Domain("ya.ru"), Domain("0.ya.ru"), Domain("2.ya.ru"),
                Domain("3.ya.ru"), Domain("4.ya.ru")
            };
            auto res = CheckDomains(banned, to_check);
            vector<bool> exp = { false, false, false, false, false };
            ASSERT_EQUAL(res, exp);
        }*/
}

void TestGoodBad(){
    stringstream ss;
    vector<bool> tst = {0, 1};
    PrintCheckResults(tst, ss);
    string res;
    ss >> res;
    ASSERT_EQUAL(res, "Bad");
    ss >> res;
    ASSERT_EQUAL(res, "Good");
    /*{
        stringstream ss;
        PrintCheckResults(vector<bool>{true, false, false}, ss);
        vector<string> v(3);
        for (int i = 0; i < 3; ++i) {
            ss >> v[i];
        }
        vector<string> expected = { "Good", "Bad", "Bad" };
        ASSERT_EQUAL(expected, v);
    }*/
}


void TestReadDomains(){
    stringstream ss_m;
    ss_m << 2;
    ss_m << "dom1";
    ss_m << "dom2";
    vector<Domain> res_m = ReadDomains(ss_m);
    ASSERT_EQUAL(res_m.size(), 2);
    auto it = res_m[0].GetParts();
    ASSERT(it.begin()!=it.end());

    stringstream ss;
    ss << "2 ya.ru mail.ru";
    auto res = ReadDomains(ss);
    vector<Domain> exp = { Domain("ya.ru"), Domain("mail.ru") };
    ASSERT_EQUAL(exp, res);

    stringstream ss2;
    ss2 << "2\nya.ru\nmail.ru";
    auto res2 = ReadDomains(ss2);
    vector<Domain> exp2 = { Domain("ya.ru"), Domain("mail.ru") };
    ASSERT_EQUAL(exp2, res2);
}



void TestAdopted() {
    {
        string s = "a b cd";
        auto res = Split(s, " ");
        vector<string_view> v{ "a", "b", "cd" };
        ASSERT_EQUAL(res, v);
    }
    {
        Domain d = Domain("mail.ya.ru");
        auto res = d.GetReversedParts();
        vector<string> resv = { res.begin(), res.end() };
        auto expected = vector<string>{ "ru", "ya", "mail" };
        ASSERT_EQUAL(resv, expected);
    }
    {
        vector<Domain> v = { Domain("mail.ya.ru") };
        DomainChecker ch(v.begin(), v.end());
        ASSERT(ch.IsSubdomain(v[0]));
        Domain d2 = Domain("ya.ru");
        ASSERT(!ch.IsSubdomain(d2));
        Domain d3 = Domain("m2.mail.ya.ru");
        ASSERT(ch.IsSubdomain(d3));
    }
    {
        stringstream ss;
        PrintCheckResults(vector<bool>{true, false, false}, ss);
        vector<string> v(3);
        for (int i = 0; i < 3; ++i) {
            ss >> v[i];
        }
        vector<string> expected = { "Good", "Bad", "Bad" };
        ASSERT_EQUAL(expected, v);
    }
    {
        vector<Domain> banned = { Domain("ya.ru"), Domain("mail.ya.ru"), Domain("mail.ru") };
        vector<Domain> to_check = {
            Domain("ya.ru"), Domain("mail.ya.ru"), Domain("mail.ru"),
            Domain("ru"), Domain(""), Domain("ba.ya.ru"), Domain("a.a.a.a.ya.ru")
        };
        auto res = CheckDomains(banned, to_check);
        vector<bool> exp = { false, false, false, true, true, false, false };
        ASSERT_EQUAL(res, exp);
    }
    {
        vector<Domain> banned = { Domain("ya.ru"), Domain("1.ya.ru"), Domain("3.ya.ru") };
        vector<Domain> to_check = {
            Domain("ya.ru"), Domain("0.ya.ru"), Domain("2.ya.ru"),
            Domain("3.ya.ru"), Domain("4.ya.ru")
        };
        auto res = CheckDomains(banned, to_check);
        vector<bool> exp = { false, false, false, false, false };
        ASSERT_EQUAL(res, exp);
    }
    {
        stringstream ss;
        ss << "2 ya.ru mail.ru";
        auto res = ReadDomains(ss);
        vector<Domain> exp = { Domain("ya.ru"), Domain("mail.ru") };
        ASSERT_EQUAL(exp, res);
    }
    {
        stringstream ss;
        ss << "2\nya.ru\nmail.ru";
        auto res = ReadDomains(ss);
        vector<Domain> exp = { Domain("ya.ru"), Domain("mail.ru") };
        ASSERT_EQUAL(exp, res);
    }
}





void TestSimple() {
  TestSplitting();
  TestReversedPart();
  TestSubdomain();
  TestAbsorption();
  TestBadSubdomain();
  TestGoodBad();
  TestReadDomains();
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSplitting);
  RUN_TEST(tr, TestReversedPart);
  RUN_TEST(tr, TestSubdomain);
  RUN_TEST(tr, TestAbsorption);
  RUN_TEST(tr, TestBadSubdomain);
  RUN_TEST(tr, TestGoodBad);
  RUN_TEST(tr, TestReadDomains);
  RUN_TEST(tr, TestAdopted);

  const vector<Domain> banned_domains = ReadDomains();
  const vector<Domain> domains_to_check = ReadDomains();
  PrintCheckResults(CheckDomains(banned_domains, domains_to_check));
  return 0;
}
