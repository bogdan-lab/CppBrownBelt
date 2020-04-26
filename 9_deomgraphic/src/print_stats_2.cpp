#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <sstream>
#include <map>
#include <set>

using namespace std;

template <typename Iterator>
class IteratorRange {
public:
  IteratorRange(Iterator begin, Iterator end)
    : first(begin)
    , last(end)
  {
  }

  Iterator begin() const {return first;}
  Iterator end() const {return last;}

private:
  Iterator first, last;
};

template <typename Collection>
auto Head(const Collection& v, size_t top) {
  return IteratorRange{v.begin(), next(v.begin(), min(top, v.size()))};
}

struct Person {
  string name;
  int age, income;
  bool is_male;
};

vector<Person> ReadPeople(istream& input) {
  int count;
  input >> count;

  vector<Person> result(count);
  for (Person& p : result) {
    char gender;
    input >> p.name >> p.age >> p.income >> gender;
    p.is_male = gender == 'M';
  }
  return result;
}

struct Demography{
    vector<Person> age_sort;
    vector<Person> wealth_sort;
    map<int, set<string>> man_names;
    map<int, set<string>> wom_names;
};


void AddNames(Demography& demography, const vector<Person>& data){
    map<string, int> tmp_man;
    map<string, int> tmp_wom;
    for(const auto& el : data){
        if(el.is_male){tmp_man[el.name]++;}
        else{tmp_wom[el.name]++;}
    }
    //now inverse maps
    for(const auto& el : tmp_man){
        demography.man_names[el.second].insert(el.first);
    }
    for(const auto& el : tmp_wom){
        demography.wom_names[el.second].insert(el.first);
    }
}



int main() {
    //test data
    stringstream ss;
    ss << "11\n"
       << "Ivan 25 1000 M\n"
       << "Olga 30 623 W\n"
       << "Sergey 24 825 M\n"
       << "Maria 42 1254 W\n"
       << "Mikhail 15 215 M\n"
       << "Oleg 18 230 M\n"
       << "Denis 53 8965 M\n"
       << "Maxim 37 9050 M\n"
       << "Ivan 47 19050 M\n"
       << "Ivan 17 50 M\n"
       << "Olga 23 550 W\n"
       << "AGE 18\n"
       << "AGE 25\n"
       << "WEALTHY 5\n"
       << "POPULAR_NAME M\n";

    vector<Person> data = ReadPeople(cin);

    const Demography dmg = [data]{
        Demography demography;
        vector<Person> age_sort = data;
        sort(begin(age_sort), end(age_sort), [](const Person& lhs, const Person& rhs) {
            return lhs.age < rhs.age;
        });
        demography.age_sort = move(age_sort);
        vector<Person> wealth_sort = data;
        sort(begin(wealth_sort), end(wealth_sort), [](const Person& lhs, const Person& rhs) {
            return lhs.income > rhs.income;
        });
        demography.wealth_sort = move(wealth_sort);
        AddNames(demography, data);
        return demography;
    }();

    for (string command; cin >> command; ) {
        if (command == "AGE") {
            int adult_age;
            cin >> adult_age;

            auto adult_begin = lower_bound(
                        begin(dmg.age_sort), end(dmg.age_sort), adult_age, [](const Person& lhs, int age) {
                return lhs.age < age;
            }
            );

            cout << "There are " << std::distance(adult_begin, end(dmg.age_sort))
                 << " adult people for maturity age " << adult_age << '\n';
        }else if (command == "WEALTHY") {
            int count;
            cin >> count;
            auto head = Head(dmg.wealth_sort, count);
            int total_income = 0;
            for (const auto& el : head){
                total_income+=el.income;
            }
            cout << "Top-" << count << " people have total income " << total_income << '\n';
        } else if (command == "POPULAR_NAME") {
            char gender;
            cin >> gender;
            std::optional<string> name;
            if (gender=='M'){
                if (dmg.man_names.size()!=0) {name = *((dmg.man_names.rbegin()->second).begin());}
            } else {
                if (dmg.wom_names.size()!=0) {name = *((dmg.wom_names.rbegin()->second).begin());}
            }
            if(name){
                cout << "Most popular name among people of gender " << gender << " is "
                             << name.value() << '\n';
            } else {
                cout << "No people of gender " << gender << '\n';
            }
        }

    }

}
