#include "test_runner.h"

#include <cstddef>  // нужно для nullptr_t

using namespace std;

// Реализуйте шаблон класса UniquePtr
template <typename T>
class UniquePtr {
private:
  T* object_ptr;
public:
  UniquePtr(): object_ptr(nullptr){}
  UniquePtr(T * ptr): object_ptr(ptr){}
  UniquePtr(const UniquePtr&) = delete;
  UniquePtr(UniquePtr&& other):object_ptr(other.Release()){}
  UniquePtr& operator = (const UniquePtr&) = delete;
  UniquePtr& operator = (nullptr_t){
      delete object_ptr;
      this->object_ptr = nullptr;
      return *this;
  }
  UniquePtr& operator = (UniquePtr&& other){
      if(this != &other){
          delete object_ptr;
          this->object_ptr = other.Release();
      }
      return *this;
  }
  ~UniquePtr(){
      delete object_ptr;
  }

  T& operator * () const{
      return *object_ptr;
  }

  T * operator -> () const{
      return object_ptr;
  }

  T * Release(){
      T* tmp = object_ptr;
      object_ptr = nullptr;
      return tmp;
  }

  void Reset(T * ptr){
      delete object_ptr;
      this->object_ptr = ptr;
  }

  void Swap(UniquePtr& other){
      T* lhs = this->object_ptr;
      this->object_ptr = other.Release();
      other.Reset(lhs);
  }

  T * Get() const {
      return object_ptr;
  }
};


struct Item {
  static int counter;
  int value;
  Item(int v = 0): value(v) {
    ++counter;
  }
  Item(const Item& other): value(other.value) {
    ++counter;
  }
  ~Item() {
    --counter;
  }
};

int Item::counter = 0;


void TestLifetime() {
  Item::counter = 0;
  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    ptr.Reset(new Item);
    ASSERT_EQUAL(Item::counter, 1);
  }
  ASSERT_EQUAL(Item::counter, 0);

  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    auto rawPtr = ptr.Release();
    ASSERT_EQUAL(Item::counter, 1);

    delete rawPtr;
    ASSERT_EQUAL(Item::counter, 0);
  }
  ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
  UniquePtr<Item> ptr(new Item(42));
  ASSERT_EQUAL(ptr.Get()->value, 42);
  ASSERT_EQUAL((*ptr).value, 42);
  ASSERT_EQUAL(ptr->value, 42);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestLifetime);
  RUN_TEST(tr, TestGetters);
}
