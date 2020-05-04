#include "Common.h"
#include <list>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <atomic>

using namespace std;

class LruCache : public ICache {
public:
  LruCache(
      shared_ptr<IBooksUnpacker> books_unpacker,
      const Settings& settings
  ): max_memory_(settings.max_memory), unpacker_(move(books_unpacker)) {
    used_memory_ = 0;
  }

  BookPtr GetBook(const string& book_name) override {
    lock_guard<mutex> g(m);
      if (cache_.count(book_name)==0){
          //not found -> unpack
          unique_ptr<IBook> book = unpacker_->UnpackBook(book_name);
          if(book->GetContent().size()>max_memory_){
              return move(book);
          }
          while(book->GetContent().size()>(max_memory_ - used_memory_)){
              used_memory_ -= rang.back()->GetContent().size();
              cache_.erase(rang.back()->GetName());
              rang.pop_back();
          }
          //add book to front
          used_memory_ += book->GetContent().size();
          rang.push_front(move(book));
          cache_[book_name] = rang.begin();
          return *(rang.begin());
      }
      else {
          BookPtr tmp_book = *cache_[book_name];
          rang.erase(cache_[book_name]);
          rang.push_front(move(tmp_book));
          cache_[book_name] = rang.begin();
          return *(rang.begin());
      }
  }

private:
  size_t used_memory_;
  const size_t max_memory_;
  shared_ptr<IBooksUnpacker> unpacker_;
  list<BookPtr> rang;
  unordered_map<string, list<BookPtr>::iterator> cache_;
  mutex m;
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
  return make_unique<LruCache>(books_unpacker, settings);
}
