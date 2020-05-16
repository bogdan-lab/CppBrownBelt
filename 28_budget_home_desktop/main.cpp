#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <ctime>
#include <list>
#include <set>
#include <iomanip>
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

    int ConvertToDayNum() const {
        const time_t timestamp = this->AsTimestamp();
        const time_t shift_timestamp = Date(2000, 1, 1).AsTimestamp();
        return (timestamp-shift_timestamp)/(60*60*24);
    }

 friend bool operator==(const Date& lhs, const Date& rhs);
 friend ostream& operator<<(ostream& out, const Date& d);
 friend bool operator<(const Date& lhs, const Date& rhs);
};


size_t ComputeDaysDiff(const Date& date_to, const Date& date_from) {
  return date_to.ConvertToDayNum() - date_from.ConvertToDayNum();
}


size_t GetDateInDayNum(istream& input){
    string s;
    input >> s;
    return Date(move(s)).ConvertToDayNum();
}

struct Command{
    Actions action_;
    size_t from_;
    size_t to_;
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
    map<size_t, double> income_;  //date_from -> money_each_day


    pair<map<size_t, double>::iterator, bool> AddPoint(size_t date){
        auto it = income_.lower_bound(date);
        if((it==income_.begin() && income_.begin()->first!=date) || it==income_.end()){
             return income_.insert({date, 0.0});
        }
        else if (it->first != date){
            return income_.insert({date, prev(it)->second});
        }
        else {
            return {it, true};
        }
    }


public:
    optional<double> ApplyCommand(Command cmd){
        if(cmd.action_==Actions::Earn){
            this->EarnMoney(cmd.from_, cmd.to_, cmd.sum_.value());
            return nullopt;
        }
        else if (cmd.action_==Actions::PayTax){
            this->PayTaxes(cmd.from_, cmd.to_);
            return nullopt;
        }
        else {
            return this->ComputeIncome(cmd.from_, cmd.to_);
        }
    }

    void EarnMoney(size_t from, size_t to, size_t sum){
        double sum_per_day = 1.0*sum/(to - from);
        //map<size_t, double>::iterator it_from;
        //map<size_t, double>::iterator it_to;
        //bool status;
        auto [it_from, status1] = AddPoint(from);
        auto [it_to, status2] = AddPoint(to);
        //auto it_from = income_.lower_bound(from);
        //auto it_to = income_.lower_bound(to);
        for(auto it=it_from; it!=it_to; it++){
            it->second+=sum_per_day;
        }
    }

    void PayTaxes(size_t from, size_t to){
        auto [it_from, status1] = AddPoint(from);
        auto [it_to, status2] = AddPoint(to);
        //auto it_from = income_.lower_bound(from);
        //auto it_to = income_.lower_bound(to);
        for(auto it=it_from; it!=it_to; it++){
            it->second*=0.87;
        }
    }

    double ComputeIncome(size_t from, size_t to){
        auto [it_from, status1] = AddPoint(from);
        auto [it_to, status2] = AddPoint(to);
        double sum=0;
        //auto it_from = income_.lower_bound(from);
        //auto it_to = income_.lower_bound(to);
        for(auto it=it_from; it!=it_to; it++){
            sum+=it->second*(next(it)->first - it->first);
        }
        return sum;
    }

};



Command ReadCommand(istream& input){
    Command cmd;
    string name;
    input >> name;
    if(name=="Earn"){
        cmd.action_ = Actions::Earn;
        cmd.from_ = GetDateInDayNum(input);
        cmd.to_ = GetDateInDayNum(input)+1;
        size_t sum;
        input >> sum;
        cmd.sum_ = sum;
    }
    else if (name=="PayTax"){
        cmd.action_ = Actions::PayTax;
        cmd.from_ = GetDateInDayNum(input);
        cmd.to_ = GetDateInDayNum(input)+1;
    }
    else if (name=="ComputeIncome") {
        cmd.action_ = Actions::ComputeIncome;
        cmd.from_ = GetDateInDayNum(input);
        cmd.to_ = GetDateInDayNum(input)+1;
    }
    else {
        throw invalid_argument("UNKNOWN COMMAND");
    }
    return cmd;
}


void ProcessCommands(istream& input, ostream& output){
    Budget bg;
    size_t count;
    input >> count;
    for(size_t i=0; i<count; i++){
        Command cmd = ReadCommand(input);
        optional<double> res = bg.ApplyCommand(move(cmd));
        if(res){
            output << setprecision(25) << res.value() << "\n";
        }
    }
}










void TestGetDate(){
    stringstream ss;
    ss << "2012-12-21";
    size_t got = GetDateInDayNum(ss);
    size_t expected = Date(2012, 12, 21).ConvertToDayNum();
    ASSERT_EQUAL(got, expected)
}

void TestConvertToDayNum(){
    {
        stringstream ss;
        ss << "2000-01-01";
        size_t got = GetDateInDayNum(ss);
        size_t expected = 0;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "2000-01-02";
        size_t got = GetDateInDayNum(ss);
        size_t expected = 1;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "2001-01-01";
        size_t got = GetDateInDayNum(ss);
        size_t expected = 366;
        ASSERT_EQUAL(got, expected);
    }

}


void TestReadCommand(){
    {
        stringstream ss;
        ss << "Earn 2000-01-02 2000-01-06 20";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::Earn;
        expected.from_ = Date("2000-01-02").ConvertToDayNum();
        expected.to_ = Date("2000-01-06").ConvertToDayNum()+1;
        expected.sum_ = 20;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "ComputeIncome 2000-01-01 2001-01-01";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::ComputeIncome;
        expected.from_ = Date("2000-01-01").ConvertToDayNum();
        expected.to_ = Date("2001-01-01").ConvertToDayNum()+1;
        expected.sum_ = nullopt;
        ASSERT_EQUAL(got, expected);
    }
    {
        stringstream ss;
        ss << "PayTax 2000-01-02 2000-01-03";
        Command got = ReadCommand(ss);
        Command expected;
        expected.action_ = Actions::PayTax;
        expected.from_ = Date("2000-01-02").ConvertToDayNum();
        expected.to_ = Date("2000-01-03").ConvertToDayNum()+1;
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


void TestComputeIncome(){
    stringstream test_input;
    test_input << 8;
    test_input << "Earn 2000-01-02 2000-01-06 20\n";
    test_input << "ComputeIncome 2000-01-01 2001-01-01\n";
    test_input << "PayTax 2000-01-02 2000-01-03\n";
    test_input << "ComputeIncome 2000-01-01 2001-01-01\n";
    test_input << "Earn 2000-01-03 2000-01-03 10\n";
    test_input << "ComputeIncome 2000-01-01 2001-01-01\n";
    test_input << "PayTax 2000-01-03 2000-01-03\n";
    test_input << "ComputeIncome 2000-01-01 2001-01-01\n";
    stringstream test_output;
    ProcessCommands(test_input, test_output);
    stringstream expected;
    expected <<"20\n18.96\n28.96\n27.2076\n";
    ASSERT_EQUAL(test_output.str(), expected.str());
}



int main(){
    /*TestRunner tr;
    RUN_TEST(tr, TestGetDate);
    RUN_TEST(tr, TestConvertToDayNum);
    RUN_TEST(tr, TestReadCommand);
    RUN_TEST(tr, TestDayDifference);
    RUN_TEST(tr, TestComputeIncome);*/

    ProcessCommands(cin, cout);
    return 0;
}


