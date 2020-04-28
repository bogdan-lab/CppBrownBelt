#include "test_runner.h"

#include <functional>
#include <string>
using namespace std;

template <typename T>
class LazyValue {
public:
    explicit LazyValue(std::function<T()> init) : func(init){}


    bool HasValue() const {return value.has_value();}
    const T& Get() const{
        if (value){return value.value();}
        else{
            value = func();
            return value.value();
        }
    }


private:
    mutable std::optional<T> value;
    std::function<T()> func;
};

void DoSomething(const vector<string>& v) {
    for (auto it = v.begin(); it != v.end(); ++it) {
        auto s = *it;
        cout << "s" << "\n";
    }
}


void UseExample() {
  const string big_string = "Giant amounts of memory";

  LazyValue<string> lazy_string([&big_string] { return big_string; });

  ASSERT(!lazy_string.HasValue());
  ASSERT_EQUAL(lazy_string.Get(), big_string);
  ASSERT_EQUAL(lazy_string.Get(), big_string);
}

void TestInitializerIsntCalled() {
  bool called = false;

  {
    LazyValue<int> lazy_int([&called] {
      called = true;
      return 0;
    });
  }
  ASSERT(!called);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, UseExample);
  RUN_TEST(tr, TestInitializerIsntCalled);
  vector<string> v = {"one", "two", "three"};
  DoSomething(v);
  return 0;
}
