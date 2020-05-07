//#include "stuff.h"
#include <iostream>

using namespace std;

void PrintStats(vector<Person> persons){
    cout << "Median age = " << ComputeMedianAge(persons.begin(), persons.end()) << "\n";
    auto male_start = partition(persons.begin(), persons.end(),
                               [](const Person& el){
                                return el.gender==Gender::FEMALE;});
    cout << "Median age for females = " <<  ComputeMedianAge(persons.begin(), male_start) << "\n";
    cout << "Median age for males = " << ComputeMedianAge(male_start, persons.end()) << "\n";
    auto fem_unempl = partition(persons.begin(), male_start,
                                [](const Person& el){return el.is_employed;});
    cout << "Median age for employed females = " << ComputeMedianAge(persons.begin(), fem_unempl) << "\n";
    cout << "Median age for unemployed females = " << ComputeMedianAge(fem_unempl, male_start) << "\n";
    auto male_unempl = partition(male_start, persons.end(),
                                 [](const Person& el){return el.is_employed;});
    cout << "Median age for employed males = " << ComputeMedianAge(male_start, male_unempl) << "\n";
    cout << "Median age for unemployed males = " << ComputeMedianAge(male_unempl, persons.end()) << "\n";
}



int main() {
  vector<Person> persons = {
      {31, Gender::MALE, false},
      {40, Gender::FEMALE, true},
      {24, Gender::MALE, true},
      {20, Gender::FEMALE, true},
      {80, Gender::FEMALE, false},
      {78, Gender::MALE, false},
      {10, Gender::FEMALE, false},
      {55, Gender::MALE, true},
  };
  PrintStats(persons);
  return 0;
}
