#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <ctime>
#include <list>
#include <set>
#include "test_runner.h"

using namespace std;


enum Actions{
    ComputeIncome,
    Earn,
    PayTax
};




class Date{
private:
    size_t year;
    size_t month;
    size_t day;
public:
    Date() = default;

    Date(size_t y, size_t m, size_t d): year(y), month(m), day(d){}

    Date(string s){
        year = stoul(s.substr(0, 4));
        month = stoul(s.substr(5,2));
        day = stoul(s.substr(8,2));
    }

    time_t AsTimestamp() const {
      std::tm t;
      t.tm_sec   = 0;
      t.tm_min   = 0;
      t.tm_hour  = 0;
      t.tm_mday  = day;
      t.tm_mon   = month - 1;
      t.tm_year  = year - 1900;
      t.tm_isdst = 0;
      return mktime(&t);
    }

 friend bool operator==(const Date& lhs, const Date& rhs);
 friend ostream& operator<<(ostream& out, const Date& d);
 friend bool operator<(const Date& lhs, const Date& rhs);
};


size_t ComputeDaysDiff(const Date& date_to, const Date& date_from) {
  const time_t timestamp_to = date_to.AsTimestamp();
  const time_t timestamp_from = date_from.AsTimestamp();
  static const size_t SECONDS_IN_DAY = 60 * 60 * 24;
  return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}


bool operator==(const Date& lhs, const Date& rhs){
    return lhs.year==rhs.year && lhs.month==rhs.month && lhs.day==rhs.day;
}


ostream& operator<<(ostream& out, const Date& d){
    out << d.year << "-" << d.month << "-" << d.day;
    return out;
}

bool operator<(const Date& lhs, const Date& rhs){
    if(lhs.year<rhs.year){
        return true;
    } else if (lhs.year>rhs.year) {
        return false;
    } else {
        if (lhs.month < rhs.month){
            return true;
        }
        else if (lhs.month > rhs.month){
            return false;
        }
        else {
            if(lhs.day < rhs.day){
                return true;
            }
            else {
                return false;
            }
        }
    }
}


Date GetDate(istream& input){
    string s;
    input >> s;
    Date d(move(s));
    return d;
}

struct Command{
    Actions action_;
    Date from_;
    Date to_;
    optional<size_t> sum_;
    Command() = default;
};


bool operator==(const Command& lhs, const Command& rhs){
    return lhs.action_==rhs.action_ && lhs.from_==rhs.from_ && lhs.to_==rhs.to_ && lhs.sum_==rhs.sum_;
}


ostream& operator<<(ostream& out, const Command& cmd){
    out << "ACT = " << cmd.action_ << "  |  FROM = " << cmd.from_ << "  |  TO = " << cmd.to_;
    if(cmd.sum_){
        out << "  |  SUM = " << cmd.sum_.value();
    }
    return out;
}



class Budget{
private:
    struct Income{
        Date from;
        Date to;
        size_t sum;
    };

    map<Date, Income> all_data_;  //from -> (to, sum_for_each_day)

    void CrossectIncomes(const list<Income>& existing, const Income& new_income){
        for(const auto& el : existing){
            if (new_income.from > el.from){
                if(el.to>new_income.from){
                    all_data_[el.from].to = new_income.from;
                }
            }
        }
    }

public:
    Budget():all_data_(){}

    optional<size_t> ApplyCommand(Command cmd){
        if(cmd.action_==Actions::Earn){
            //do earn
            return nullopt;
        }
        else if (cmd.action_==Actions::PayTax){
            //do tax
            return nullopt;
        }
        else {
            //do compute income
            return 1993;
        }
    }

    void EarnMoney(const Date& from, const Date& to, size_t sum){
        size_t sum_per_day = sum/(ComputeDaysDiff(to, from)+1);     //+1 since [from, to]
        auto it_from = income_.lower_bound(from);
        auto it_to = income_.lower_bound(to);
        list<Income> possible_crossings;
        for(auto it=it_from; it!=next(it_to); it++){
            possible_crossings.push_back(it->second);
        }

    }
};



Command ReadCommand(istream& input){
    Command cmd;
    string name;
    input >> name;
    if(name=="Earn"){
        cmd.action_ = Actions::Earn;
        cmd.from_ = GetDate(input);
        cmd.to_ = GetDate(input);
        size_t sum;
        input >> sum;
        cmd.sum_ = sum;
    }
    else if (name=="PayTax"){
        cmd.action_ = Actions::PayTax;
        cmd.from_ = GetDate(input);
        cmd.to_ = GetDate(input);
    }
    else if (name=="ComputeIncome") {
        cmd.action_ = Actions::ComputeIncome;
        cmd.from_ = GetDate(input);
        cmd.to_ = GetDate(input);
    }
    else {
        throw invalid_argument("UNKNOWN COMMAND");
    }
    return cmd;
}


void ProcessCommands(istream& input){
    size_t count;
    input >> count;
    for(size_t i=0; i<count; i++){
        Command cmd = ReadCommand(input);
    }
}






void TestGetDate(){
    stringstream ss;
    ss << "2012-12-21";
    Date d = GetDate(ss);
    Date expected = {2012, 12, 21};
    ASSERT_EQUAL(d, expected)
}

void TestReadCommand(){
    {
        stringstream ss;
        ss << "Earn 2000-01-02 2000-01-06 20";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::Earn;
        expected.from_ = Date("2000-01-02");
        expected.to_ = Date(Date("2000-01-06"));
        expected.sum_ = 20;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "ComputeIncome 2000-01-01 2001-01-01";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::ComputeIncome;
        expected.from_ = Date("2000-01-01");
        expected.to_ = Date("2001-01-01");
        expected.sum_ = nullopt;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "PayTax 2000-01-02 2000-01-03";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::PayTax;
        expected.from_ = Date("2000-01-02");
        expected.to_ = Date("2000-01-03");
        expected.sum_ = nullopt;
        ASSERT_EQUAL(got, expected);
    }
}


void TestDayDifference(){
    {
        Date from("2000-01-01");
        Date to("2000-01-05");
        size_t got = ComputeDaysDiff(to, from);
        size_t expected =4;
        ASSERT_EQUAL(got, expected);
    }
    {
        Date from("2000-01-05");
        Date to("2000-01-05");
        size_t got = ComputeDaysDiff(to, from);
        size_t expected =0;
        ASSERT_EQUAL(got, expected);
    }
}


void TestDateCompare(){
    set<Date> date_set;
    date_set.insert(Date("2000-12-25"));
    date_set.insert(Date("2000-11-15"));
    date_set.insert(Date("2001-01-02"));
    date_set.insert(Date("2000-11-15"));
    ASSERT_EQUAL(date_set.size(), 3);
    Date expected_begin("2000-11-15");
    ASSERT_EQUAL(*(date_set.begin()), expected_begin);
    Date expected_end("2001-01-02");
    ASSERT_EQUAL(*(prev(date_set.end())), expected_end);
}


int main(){
    TestRunner tr;
    RUN_TEST(tr, TestGetDate);
    RUN_TEST(tr, TestReadCommand);
    RUN_TEST(tr, TestDayDifference);
    RUN_TEST(tr, TestDateCompare);
    return 0;
}


