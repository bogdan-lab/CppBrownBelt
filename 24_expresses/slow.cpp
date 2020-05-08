#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;


class RouteManager{
private:
    unordered_map<int, set<int>> from_to_;
public:
    void AddRoute(int start, int finish) {
        from_to_[start].insert(finish);
        from_to_[finish].insert(start);
    }

    int FindNearestFinish(int start, int finish) const {
        vector<int> possible_distances = {abs(start-finish)};
        if(from_to_.count(start)!=0){
            auto lbd_it =  from_to_.at(start).lower_bound(finish);
            if (lbd_it==from_to_.at(start).end()){
                possible_distances.push_back(abs(finish - *(--lbd_it)));
            }
            else if (lbd_it==from_to_.at(start).begin()){
                possible_distances.push_back(abs(*lbd_it-finish));
            }
            else {
                possible_distances.push_back(abs(*lbd_it-finish));
                possible_distances.push_back(abs(finish - *(--lbd_it)));
            }
        }
        return *min_element(possible_distances.begin(), possible_distances.end());
    }

};


int main() {
  RouteManager routes;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int start, finish;
    cin >> start >> finish;
    if (query_type == "ADD") {
      routes.AddRoute(start, finish);
    } else if (query_type == "GO") {
      cout << routes.FindNearestFinish(start, finish) << "\n";
    }
  }

  return 0;
}
