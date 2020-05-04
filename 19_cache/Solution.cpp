#include "Common.h"
#include <list>
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
    current_memory_ = 0;
  }

  BookPtr GetBook(const string& book_name) override {
    lock_guard<mutex> g(m);
      //find book in cache
      auto filter = [book_name](pair<string, BookPtr> el){
        return book_name==el.first;
      };
      auto it = find_if(cache_.begin(), cache_.end(), filter);
      if (it == cache_.end()){
          //not found -> unpack
          unique_ptr<IBook> book = unpacker_->UnpackBook(book_name);
          if(book->GetContent().size()>max_memory_){
              return move(book);
          }
          while(book->GetContent().size()>(max_memory_ - current_memory_)){
              current_memory_ -= cache_.back().second->GetContent().size();
              cache_.pop_back();
          }
          //add book to front
          cache_.push_front(make_pair(book_name, move(book)));
          return cache_.front().second;
      }
      else {
          return it->second;
      }
  }

private:
  atomic<size_t> current_memory_;
  const size_t max_memory_;
  shared_ptr<IBooksUnpacker> unpacker_;
  list<pair<string, BookPtr>> cache_;
  mutex m;
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
  return make_unique<LruCache>(books_unpacker, settings);
}
