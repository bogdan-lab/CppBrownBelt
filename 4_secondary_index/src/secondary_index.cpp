#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Record {
  string id;
  string title;
  string user;
  int timestamp;
  int karma;
};

// Реализуйте этот класс
class Database {
public:
    bool Put(const Record& record){
        if(data_base.count(record.id)==0){
            auto s1 = data_base.insert(make_pair(record.id, record));
            if(s1.second){
                m_user[record.user].insert(record.id);
                m_karma[record.karma].insert(record.id);
                m_tstmp[record.timestamp].insert(record.id);
                return true;
            }
        }
        return false;
    }

    const Record* GetById(const string& id) const{
        if(data_base.count(id)>0){
            return &data_base.at(id);
        }
        return nullptr;
    }

    bool Erase(const string& id){
        if(data_base.count(id)>0){
            Record tmp = data_base[id];
            m_user[tmp.user].erase(id);
            m_karma[tmp.karma].erase(id);
            m_tstmp[tmp.timestamp].erase(id);
            data_base.erase(id);
            return true;
        }
        return false;
    }

  template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const{
        for(int time=low; time<=high; time++){
            if(m_tstmp.count(time)>0){
                auto it = m_tstmp.at(time).begin();
                while((it!=m_tstmp.at(time).end()) && (callback(data_base.at(*it)))){
                    it++;
                }
            }
        }
    }

  template <typename Callback>
  void RangeByKarma(int low, int high, Callback callback) const{
      for(int karma=low; karma<=high; karma++){
          if(m_karma.count(karma)>0){
              auto it = m_karma.at(karma).begin();
              while((it!=m_karma.at(karma).end()) && (callback(data_base.at(*it)))){
                  it++;
              }
          }
      }
  }

  template <typename Callback>
  void AllByUser(const string& user, Callback callback) const{
      auto it = m_user.at(user).begin();
      while((it!=m_user.at(user).end()) && (callback(data_base.at(*it)))){
          it++;
      }
  }


private:
  unordered_map<int, set<string>> m_tstmp;      //time_stamp -> id
  unordered_map<int, set<string>> m_karma;      //karma -> id
  unordered_map<string, set<string>> m_user;    //user -> id
  unordered_map<string, Record> data_base;      //id -> record
};

void TestRangeBoundaries() {
  const int good_karma = 1000;
  const int bad_karma = -10;

  Database db;
  db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
  db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

  int count = 0;
  db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestSameUser() {
  Database db;
  db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
  db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

  int count = 0;
  db.AllByUser("master", [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestReplacement() {
  const string final_body = "Feeling sad";

  Database db;
  db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
  db.Erase("id");
  db.Put({"id", final_body, "not-master", 1536107260, -10});

  auto record = db.GetById("id");
  ASSERT(record != nullptr);
  ASSERT_EQUAL(final_body, record->title);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestRangeBoundaries);
  RUN_TEST(tr, TestSameUser);
  RUN_TEST(tr, TestReplacement);
  return 0;
}
