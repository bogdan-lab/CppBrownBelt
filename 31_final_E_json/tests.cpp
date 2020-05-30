#include <sstream>
#include <fstream>
#include <iomanip>
#include "tests.h"
#include "test_runner.h"
#include "bus_catalog.h"
#include "requests.h"
#include "json.h"



using namespace std;



void TestJsonRead(){
    using namespace Json;
    ifstream inFile;
    inFile.open("./read_test");
    Document doc = Load(inFile);
    inFile.close();
    map<string, Node> all_requests = doc.GetRoot().AsMap();
    {
    vector<Node> base_requests = all_requests.at("base_requests").AsArray();
    ASSERT_EQUAL(base_requests.size(), 4);
    map<string, Node> req_0 = base_requests[0].AsMap();
    ASSERT_EQUAL(req_0.at("type").AsString(), "Stop");
    ASSERT_EQUAL(req_0.at("name").AsString(), "Tolstopaltsevo");
    map<string, Node> req_0_roads = req_0.at("road_distances").AsMap();
    ASSERT_EQUAL(req_0_roads.size(), 1);
    ASSERT_EQUAL(req_0_roads.at("Marushkino").DoubleAsInt(), 3900);
    ASSERT_EQUAL(req_0.at("longitude").AsDouble(), 37.20829);
    ASSERT_EQUAL(req_0.at("latitude").AsDouble(), 55.611087);


    map<string, Node> req_1 = base_requests[1].AsMap();
    ASSERT_EQUAL(req_1.at("type").AsString(), "Bus");
    ASSERT_EQUAL(req_1.at("name").AsString(), "256");
    ASSERT_EQUAL(req_1.at("is_roundtrip").AsBool(), true);
    vector<Node> stops = req_1.at("stops").AsArray();
    ASSERT_EQUAL(stops.size(), 6);
    ASSERT_EQUAL(stops[0].AsString(), "Biryulyovo Zapadnoye");

    map<string, Node> req_2 = base_requests[2].AsMap();
    ASSERT_EQUAL(req_2.at("type").AsString(), "Stop");
    ASSERT_EQUAL(req_2.at("road_distances").AsMap().empty(), true);
    ASSERT_EQUAL(req_2.at("name").AsString(), "Rasskazovka");
    ASSERT_EQUAL(req_2.at("longitude").AsDouble(), 37);
    ASSERT_EQUAL(req_2.at("latitude").AsDouble(), 55);

    map<string, Node> req_3 = base_requests[3].AsMap();
    ASSERT_EQUAL(req_3.at("is_roundtrip").AsBool(), false);
    }
    {
    vector<Node> stat_requests = all_requests.at("stat_requests").AsArray();
    ASSERT_EQUAL(stat_requests.size(), 2);
    map<string, Node> req_0 = stat_requests[0].AsMap();
    ASSERT_EQUAL(req_0.at("type").AsString(), "Bus");
    ASSERT_EQUAL(req_0.at("name").AsString(), "256");
    ASSERT_EQUAL(req_0.at("id").DoubleAsInt(), 0);

    map<string, Node> req_1 = stat_requests[1].AsMap();
    ASSERT_EQUAL(req_1.at("type").AsString(), "Stop");
    ASSERT_EQUAL(req_1.at("name").AsString(), "Biryulyovo Zapadnoye");
    ASSERT_EQUAL(req_1.at("id").DoubleAsInt(), 2147483647);

    }
}



void RunTests(){

    TestRunner tr;
    RUN_TEST(tr, TestJsonRead);

}










