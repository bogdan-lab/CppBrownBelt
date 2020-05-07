#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <algorithm>

using namespace std;


class ReadingManager {
private:
    static const int MAX_PAGES_ = 1000;
    vector<int> page_to_user_num;
    unordered_map<int, int> user_to_page;
public:
    ReadingManager(): page_to_user_num(MAX_PAGES_, 0), user_to_page(){}

    void Read(int user_id, int page_count) {
        if (user_to_page.count(user_id)!=0) {
            int old_page = user_to_page[user_id];
            page_to_user_num[old_page]--;
        }
        page_to_user_num[page_count]++;
        user_to_page[user_id] = page_count;
    }

    double Cheer(int user_id) const {
        if (user_to_page.count(user_id)==0) {
            return 0;
        }
        if (user_to_page.size()==1){
            return 1;
        }
        int read_pages = user_to_page.at(user_id);
        int lower = 0;
        for(int i=0; i<read_pages; i++){
            lower+=page_to_user_num[i];
        }
        return 1.0*lower/(user_to_page.size()-1);
    }
};



int main() {
  // Для ускорения чтения данных отключается синхронизация
  // cin и cout с stdio,
  // а также выполняется отвязка cin от cout
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  ReadingManager manager;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int user_id;
    cin >> user_id;

    if (query_type == "READ") {
      int page_count;
      cin >> page_count;
      manager.Read(user_id, page_count);
    } else if (query_type == "CHEER") {
      cout << setprecision(6) << manager.Cheer(user_id) << "\n";
    }
  }

  return 0;
}
