#include <sstream>
#include <fstream>
#include <iomanip>
#include "tests.h"
#include "test_runner.h"
#include "bus_catalog.h"
#include "utils.h"
#include "requests.h"
#include "json.h"



using namespace std;


void TestSavingExtraStops(){
    stringstream in;
    in << 1 <<"\n";
    in << "Stop Fancy: 1, 2, 10m to N1, 20m to N2, 30m to N3\n";
    BusCatalog bc;
    ReadInputData(bc, in);
    ASSERT_EQUAL(6, bc.GetDistStructSize());
}


void TestNewFormatStops(){
    stringstream in;
    in <<"13\n";
    in <<"Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n";
    in <<"Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n";
    in <<"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
    in <<"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n";
    in <<"Stop Rasskazovka: 55.632761, 37.333324\n";
    in <<"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n";
    in <<"Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n";
    in <<"Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n";
    in <<"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n";
    in <<"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n";
    in <<"Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n";
    in <<"Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n";
    in <<"Stop Prazhskaya: 55.611678, 37.603831\n";
    in <<"6\n";
    in <<"Bus 256\n";
    in <<"Bus 750\n";
    in <<"Bus 751\n";
    in <<"Stop Samara\n";
    in <<"Stop Prazhskaya\n";
    in <<"Stop Biryulyovo Zapadnoye\n";
    BusCatalog catalog;
    ReadInputData(catalog, in);
    stringstream out;
    out<<setprecision(7);
    ProcessRequests(catalog, in, out);
    vector<string>expected ={"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.361239 curvature",
                  "Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.318084 curvature",
                  "Bus 751: not found",
                      "Stop Samara: not found",
                  "Stop Prazhskaya: no buses",
                  "Stop Biryulyovo Zapadnoye: buses 256 828"};
    for(size_t i=0; i<expected.size(); i++){
        string got;
        getline(out, got);
        ASSERT_EQUAL(expected[i], got);
    }

}


void TestJson(){
    using namespace Json;

    ifstream inFile;
    inFile.open("./input");
    if(!inFile){
        cout << "FILE WAS NOT OPEN\n";
        exit(1);
    }
    Json::Document doc = Json::Load(inFile);
    inFile.close();
    map<string, Node> all_requests = doc.GetRoot().AsMap();
    for(auto el : all_requests){
        cout << el.first << endl;
    }

    vector<Node> input_requests = all_requests.at("base_requests").AsArray();
    for(size_t i=0; i<input_requests.size(); i++){
        map<string, Node> this_request = input_requests[i].AsMap();
        if(this_request.at("type").AsString() == "Bus"){
            cout << "IS_ROUND_TRIP = "
                 << this_request.at("is_roundtrip").AsBool()
                 << endl;
        }
        cout << "TYPE -> " << this_request.at("type").AsString() << endl;
    }
    cout << "READ FILE OK\n";
}


void TestJsonRead(){
    using namespace Json;
    ifstream inFile;
    inFile.open("./read_test");
    Document doc = Load(inFile);
    inFile.close();
    map<string, Node> all_requests = doc.GetRoot().AsMap();
    vector<Node> base_requests = all_requests.at("base_requests").AsArray();
    ASSERT_EQUAL(base_requests.size(), 4);
    map<string, Node> req_0 = base_requests[0].AsMap();
    ASSERT_EQUAL(req_0.at("type").AsString(), "Stop");
    ASSERT_EQUAL(req_0.at("name").AsString(), "Tolstopaltsevo");
    map<string, Node> req_0_roads = req_0.at("road_distances").AsMap();
    ASSERT_EQUAL(req_0_roads.size(), 1);
    ASSERT_EQUAL(req_0_roads.at("Marushkino").AsInt(), 3900);
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



void RunTests(){

    TestRunner tr;
    RUN_TEST(tr, TestSavingExtraStops);
    RUN_TEST(tr, TestNewFormatStops);
    RUN_TEST(tr, TestJsonRead);

}










