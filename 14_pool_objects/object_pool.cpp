#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>
#include <list>
using namespace std;

template <class T>
class ObjectPool {
public:
    T* Allocate(){
        T* el = TryAllocate();
        if(el==nullptr){
            el = new T;
            alloc.insert(el);
        }
        return el;
    }

    T* TryAllocate(){
        if(!dealloc.empty()){
            auto status = alloc.insert(dealloc.front());
            dealloc.pop();
            return *status.first;
        }
        return nullptr;
    }

  void Deallocate(T* object){
      auto it = alloc.find(object);
      if(it==alloc.end()){
          throw std::invalid_argument("invalid argument");
      }
      dealloc.push(*it);
      alloc.erase(it);
  }

  ~ObjectPool(){
      for (T* el : alloc){
          delete el;
      }
      while(!dealloc.empty()){
        delete dealloc.front();
        dealloc.pop();
      }
  }

private:
  set<T*> alloc;
  queue<T*> dealloc;
};

void TestObjectPool() {
  ObjectPool<string> pool;

  auto p1 = pool.Allocate();
  auto p2 = pool.Allocate();
  auto p3 = pool.Allocate();

  *p1 = "first";
  *p2 = "second";
  *p3 = "third";

  pool.Deallocate(p2);
  ASSERT_EQUAL(*pool.Allocate(), "second");

  pool.Deallocate(p3);
  pool.Deallocate(p1);
  ASSERT_EQUAL(*pool.Allocate(), "third");
  ASSERT_EQUAL(*pool.Allocate(), "first");

  pool.Deallocate(p1);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestObjectPool);
  return 0;
}
