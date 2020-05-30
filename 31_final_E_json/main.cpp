#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <list>
#include <set>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <fstream>

#include "tests.h"
#include "json.h"
#include "bus_catalog.h"
#include "requests.h"


using namespace std;





int main(){
//    RunTests();

    ifstream inFile;
    inFile.open("./test");
    Json::Document doc = Json::Load(inFile);
    inFile.close();
    BusCatalog catalog;

    map<string, Json::Node> all_requests = doc.GetRoot().AsMap();
    ReadInputData(catalog, all_requests.at("base_requests").AsArray());
    cout << setprecision(6);
    ProcessRequests(catalog, all_requests.at("stat_requests").AsArray());

    return 0;
}
