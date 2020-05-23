#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <algorithm>

//#include "test_runner.h"


using namespace std;


string_view DeleteSpaces(string_view s){
    while(s.back()==' '){s.remove_suffix(1);}
    while(s.front()==' '){s.remove_prefix(1);}
    return s;
}

string_view GetUntilSplitter(string_view& s, char el){
    size_t pos = s.find(el);
    string_view info;
    if (pos != string::npos){
        info = s.substr(0, pos);
        s.remove_prefix(pos+1);
    }else {s.swap(info);}
    return DeleteSpaces(info);
}

double ConvertToDouble(string_view s){
    string_view tmp = DeleteSpaces(s);
    size_t pos;
    return stod(string(tmp), &pos);
}

struct Stop{
    string_view name_;
    double latitude_;
    double longitude_;

    Stop(string_view n, double lat, double lon):
        name_(move(n)), latitude_(move(lat)), longitude_(move(lon)){}
};



struct BusInfo{
    enum Type{
        ROUND,
        STRAIGHT,
    };

    string_view id_;
    Type type_;
    list<string_view> stop_names_;
    size_t unique_stops_ = 0;
    double route_len_ = 0.0;

    BusInfo(string_view id, Type type, list<string_view> snames):
        id_(id), type_(type), stop_names_(move(snames)) {}
};

class BusCatalog{
    using StopStruct = unordered_map<string_view, unique_ptr<Stop>>;
    using BusStruct = unordered_map<string_view, unique_ptr<BusInfo>>;

private:
    const double PI = 3.1415926535;
    const double R_EARTH = 6371000;     //m

    unordered_set<string> bus_names_;
    unordered_set<string> stop_names_;
    StopStruct stops_;
    BusStruct buses_;

    string_view SaveStopName(string_view new_stop_name){
        auto res = stop_names_.insert(string(new_stop_name));
        return *(res.first);
    }

    string_view SaveBusName(string_view new_bus_name){
        auto res = bus_names_.insert(string(new_bus_name));
        return *(res.first);
    }

    double ComputeDistance(string_view lhs, string_view rhs) const {
        const Stop *left = stops_.at(lhs).get();
        const Stop *right = stops_.at(rhs).get();
        double cos_alpha = sin(left->latitude_)*sin(right->latitude_) +
                cos(left->latitude_)*cos(right->latitude_)*(sin(left->longitude_)*sin(right->longitude_) +
                                                          cos(left->longitude_)*cos(right->longitude_));
        return R_EARTH*acos(cos_alpha);     //in m
    }

    double ComputeBusRouteLen(const BusInfo* bus) const {
        double length = 0.0;
        for(auto it=bus->stop_names_.begin(); it!=prev(bus->stop_names_.end()); it++){
            auto lhs = it;
            auto rhs = next(it);
            length +=this->ComputeDistance(*lhs, *rhs);
        }
        if(bus->type_==BusInfo::Type::STRAIGHT){
            return 2*length;
        }
        else{
            length+= this->ComputeDistance(bus->stop_names_.back(), bus->stop_names_.front());
            return length;
        }
    }

    size_t CalcUniqueStops(const BusInfo* bus) const {
        unordered_set<string_view> unique;
        for(const auto& el : bus->stop_names_){
            unique.insert(el);
        }
        return unique.size();
    }

    void UpdateBusRouteLens(){
        for(const auto& name : bus_names_){
            buses_[name]->route_len_ = ComputeBusRouteLen(buses_[name].get());
        }
    }

    void UpdateBusUniqueStops(){
        for(const auto& name : bus_names_){
            buses_[name]->unique_stops_ = CalcUniqueStops(buses_[name].get());
        }
    }

public:
    void AddBusStop(unique_ptr<Stop> new_stop){
        new_stop->name_ = SaveStopName(new_stop->name_);
        new_stop->latitude_ *= PI/180.0;
        new_stop->longitude_ *= PI/180.0;
        stops_[new_stop->name_] = move(new_stop);
    }

    void AddBus(unique_ptr<BusInfo> new_bus){
        new_bus->id_ = SaveBusName(new_bus->id_);
        for(auto& el : new_bus->stop_names_){
            el = SaveStopName(el);
        }
        buses_[new_bus->id_] = move(new_bus);
    }

    void UpdateBusInfo(){
        UpdateBusRouteLens();
        UpdateBusUniqueStops();
    }

    const Stop* GetStop(string_view stop_key) const {
        return stops_.at(stop_key).get();
    }

    const BusInfo* GetBus(string_view bus_id) const {
        auto it = buses_.find(bus_id);
        if (it!=buses_.end()){
            return it->second.get();
        }
        return nullptr;
    }

};


unique_ptr<Stop> ReadStop(string_view request){
    string_view stop_name = GetUntilSplitter(request, ':');
    double latitude = ConvertToDouble(GetUntilSplitter(request, ','));
    double longitude = ConvertToDouble(request);
    return make_unique<Stop>(Stop(stop_name, latitude, longitude));
}

BusInfo::Type GetBusType(string_view s){
    string_view check_str = s.substr(0, 28);
    size_t status = check_str.find('-');
    if(status!=string::npos){
        return BusInfo::Type::STRAIGHT;
    }
    return BusInfo::Type::ROUND;
}

char GetSplitter(BusInfo::Type type){
    if (type==BusInfo::ROUND){
        return '>';
    }
    return '-';
}

list<string_view> ReadBusStops(string_view request, char splitter){
    list<string_view> stop_names;
    while(!request.empty()){
        stop_names.push_back(GetUntilSplitter(request, splitter));
    }
    return stop_names;
}

unique_ptr<BusInfo> ReadBus(string_view request){
    string_view id = GetUntilSplitter(request, ':');
    BusInfo::Type type = GetBusType(request);
    char splitter = GetSplitter(type);
    list<string_view> stop_names = ReadBusStops(request, splitter);
    if(type==BusInfo::Type::ROUND){stop_names.pop_back();}
    return make_unique<BusInfo>(BusInfo(id, type, move(stop_names)));
}


size_t ReadRequestCount(istream& in_stream){
    size_t request_count;
    in_stream >> request_count;
    string dummy;
    getline(in_stream, dummy);
    return request_count;
}


void ReadInputData(BusCatalog& catalog, istream& in_stream=cin){
    size_t request_count = ReadRequestCount(in_stream);
    for (size_t i=0; i< request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view data_name = GetUntilSplitter(request_view, ' ');
        if(data_name=="Bus"){
            unique_ptr<BusInfo> bus = ReadBus(request_view);
            catalog.AddBus(move(bus));
        } else {
            unique_ptr<Stop> stop = ReadStop(request_view);
            catalog.AddBusStop(move(stop));
        }
    }
    catalog.UpdateBusInfo();
}



struct BusReply{
    string_view id_;
    optional<size_t> num_stops_;
    optional<size_t> unique_stops_;
    optional<double> route_len_;

    optional<size_t> GetAllStopsNum(const BusInfo* bus){
        if(bus){
            if(bus->type_==BusInfo::Type::STRAIGHT){return 2*bus->stop_names_.size()-1;}
            else {return bus->stop_names_.size()+1;}
        }
        return nullopt;
    }

    optional<size_t> GetUniqueStopsNum(const BusInfo* bus){
        if(bus){
            return bus->unique_stops_;
        }
        return nullopt;
    }

    optional<double> GetRouteLength(const BusInfo* bus){
        if(bus){
            return bus->route_len_;
        }
        return nullopt;
    }

    BusReply(const BusCatalog& catalog, string_view bus_id): id_(bus_id){
        const BusInfo* bus = catalog.GetBus(bus_id);
        num_stops_ = GetAllStopsNum(bus);
        unique_stops_ = GetUniqueStopsNum(bus);
        route_len_ = GetRouteLength(bus);
    }

    void Reply(ostream& out_stream){
        if(num_stops_){     //not beautiful way to check.....
            out_stream << "Bus " << id_ <<": "<< num_stops_.value() << " stops on route, "
                       << unique_stops_.value() << " unique stops, " <<route_len_.value() << " route length\n";
        } else {
            out_stream << "Bus " << id_ <<": not found\n";
        }
    }
};


void ProcessRequests(const BusCatalog& catalog, istream& in_stream=cin, ostream& out_stream=cout){
    size_t request_count = ReadRequestCount(in_stream);
    for(size_t i=0; i<request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view request_name = GetUntilSplitter(request_view, ' ');
        string_view bus_id = request_view;
        BusReply reply(catalog, bus_id);
        reply.Reply(out_stream);
    }
}



//-------------TESTS------------------------
/*


void TestParcingRequest(){
    string_view test = "Stop Tolstopaltsevo: 55.611087, 37.20829";
    ASSERT_EQUAL("Stop", GetUntilSplitter(test, ' '));
    ASSERT_EQUAL("Tolstopaltsevo", GetUntilSplitter(test, ':'));
    string_view tmp = GetUntilSplitter(test, ',');
    ASSERT_EQUAL("55.611087", tmp);
    ASSERT_EQUAL(55.611087, ConvertToDouble(tmp));
    ASSERT_EQUAL(37.20829, ConvertToDouble(test));

    test = "Bus 750: N1 - Stop 2 - Stop number 3";
    ASSERT_EQUAL("Bus", GetUntilSplitter(test, ' '));
    tmp = GetUntilSplitter(test, ':');
    ASSERT_EQUAL("750", tmp);
    ASSERT_EQUAL(BusInfo::Type::STRAIGHT, GetBusType(test));
    ASSERT_EQUAL(BusInfo::Type::ROUND, GetBusType("test > test > test"));
    ASSERT_EQUAL("N1", GetUntilSplitter(test, '-'));
    ASSERT_EQUAL("Stop 2", GetUntilSplitter(test, '-'));
    ASSERT_EQUAL("Stop number 3", GetUntilSplitter(test, '-'));
}


void TestReadStops(){
    double PI = 3.1415926535;
    BusCatalog bc;
    {
        stringstream ss;
        ss << 2 << "\n";
        ss << "Stop Stop N1: 90.0, 45.0\n";
        ss << "Stop               S T O P            : 90.0, 90.0\n";
        ReadInputData(bc, ss);
    }
    const auto stop1 = bc.GetStop("Stop N1");
    ASSERT_EQUAL("Stop N1", stop1->name_);
    ASSERT_EQUAL(0.5*PI, stop1->latitude_);
    ASSERT_EQUAL(0.25*PI, stop1->longitude_);
    const auto stop2 = bc.GetStop("S T O P");
    ASSERT_EQUAL("S T O P", stop2->name_);
    ASSERT_EQUAL(0.5*PI, stop2->latitude_);
    ASSERT_EQUAL(0.5*PI, stop2->longitude_);
}


void TestReadBuses(){
    const double R_EARTH = 6371000;
    const double PI = 3.1415926535;
    BusCatalog bc;
    {
        stringstream ss;
        ss << 6 << "\n";
        ss << "Stop N1: 90.0, 90.0\n";
        ss << "Stop N2: 90.0, 45.0\n";
        ss << "Stop N3: 45.0, 45.0\n";
        ss << "Stop N4: 45.0, 90.0\n";

        ss << "Bus straight  : N1 - N2 - N3 - N4\n";
        ss << "Bus circle  : N1 > N2 > N1\n";
        ReadInputData(bc, ss);
    }

    const auto bus1 = bc.GetBus("straight");
    ASSERT_EQUAL(bus1->id_, "straight");
    ASSERT_EQUAL(bus1->type_, BusInfo::Type::STRAIGHT);
    list<string_view> expected1 = {"N1", "N2", "N3", "N4"};
    ASSERT_EQUAL(bus1->stop_names_, expected1);
    //ASSERT_EQUAL(bus1->route_len_, 2*PI*R_EARTH);
    const auto bus2 = bc.GetBus("circle");
    ASSERT_EQUAL(bus2->id_, "circle");
    ASSERT_EQUAL(bus2->type_, BusInfo::Type::ROUND);
    list<string_view> expected2 = {"N1", "N2"};
    ASSERT_EQUAL(bus2->stop_names_, expected2);
    //ASSERT_EQUAL(bus2->route_len_, 0.5*PI*R_EARTH);

}


void TestBusReply(){
    stringstream ss;
    ss << 10 << "\n";
    ss << "Stop Tolstopaltsevo: 55.611087, 37.20829\n";
    ss << "Stop Marushkino: 55.595884, 37.209755\n";
    ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
    ss << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n";
    ss << "Stop Rasskazovka: 55.632761, 37.333324\n";
    ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n";
    ss << "Stop Biryusinka: 55.581065, 37.64839\n";
    ss << "Stop Universam: 55.587655, 37.645687\n";
    ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n";
    ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";
    BusCatalog catalog;
    ReadInputData(catalog, ss);
    ss << 3 << "\n";
    ss << "Bus 256\n";
    ss << "Bus 750\n";
    ss << "Bus 751\n";
    stringstream out;
    ProcessRequests(catalog, ss, out);
    string reply_1;
    getline(out, reply_1);
    string expected_1 = "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length";
    ASSERT_EQUAL(expected_1, reply_1);
    string reply_2;
    getline(out, reply_2);
    string expected_2 = "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length";
    ASSERT_EQUAL(expected_2, reply_2);
    string reply_3;
    getline(out, reply_3);
    string expected_3 = "Bus 751: not found";
    ASSERT_EQUAL(expected_3, reply_3);
}

*/




int main(){
//    TestRunner tr;
//    RUN_TEST(tr, TestParcingRequest);
//    RUN_TEST(tr, TestReadStops);
//    RUN_TEST(tr, TestReadBuses);
//    RUN_TEST(tr, TestBusReply);


    BusCatalog catalog;
    ReadInputData(catalog);
    cout << setprecision(6);
    ProcessRequests(catalog);

    return 0;
}
