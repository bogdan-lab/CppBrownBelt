#include <iostream>
#include <unordered_map>
#include <list>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include "test_runner.h"


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

int ConvertToInt(string_view s){
    string_view tmp = DeleteSpaces(s);
    size_t pos;
    return stoi(string(tmp), &pos);
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

    int id_;
    Type type_;
    list<string_view> stop_names_;

    BusInfo(int id, Type type, list<string_view> snames):
        id_(id), type_(type), stop_names_(move(snames)) {}
};

class BusCatalog{
    using StopStruct = unordered_map<string_view, unique_ptr<Stop>>;
    using BusStruct = unordered_map<int, unique_ptr<BusInfo>>;

private:
    const double PI = 3.1415926535;
    const double R_EARTH = 6371;     //km

    unordered_set<string> stop_names_;
    StopStruct stops_;
    BusStruct buses_;

    string_view SaveStopName(string_view new_stop_name){
        auto res = stop_names_.insert(string(new_stop_name));
        return *(res.first);
    }

public:
    void AddBusStop(unique_ptr<Stop> new_stop){
        string_view rebinded_name = SaveStopName(new_stop->name_);
        new_stop->name_ = rebinded_name;
        new_stop->latitude_ *= 180.0/PI;
        new_stop->longitude_ *= 180.0/PI;
        stops_[rebinded_name] = move(new_stop);
    }

    void AddBus(unique_ptr<BusInfo> new_bus){
        for(auto& el : new_bus->stop_names_){
            el = SaveStopName(el);
        }
        buses_[new_bus->id_] = move(new_bus);
    }

    double ComputeDistance(string_view lhs, string_view rhs) const {
        const Stop *left = stops_.at(lhs).get();
        const Stop *right = stops_.at(rhs).get();
        double cos_alpha = sin(left->latitude_)*sin(right->latitude_) +
                cos(left->latitude_)*cos(right->latitude_)*(sin(left->longitude_)*sin(right->longitude_) +
                                                          cos(left->longitude_)*cos(right->longitude_));
        return R_EARTH*acos(cos_alpha);
    }

    const Stop* GetStops(string_view stop_key) const {
        return stops_.at(stop_key).get();
    }

    const BusInfo* GetBus(int bus_id) const {
        return buses_.at(bus_id).get();
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
    int id = ConvertToInt(GetUntilSplitter(request, ':'));
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


void ReadInputData(BusCatalog& catalog, istream& in_stream){
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
}


vector<int> ReadRequests(istream& in_stream){
    size_t request_count = ReadRequestCount(in_stream);
    vector<int> bus_ids;
    bus_ids.reserve(request_count);
    for(size_t i=0; i<request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view request_name = GetUntilSplitter(request_view, ' ');
        bus_ids.push_back(ConvertToInt(request_view));
    }
    return bus_ids;
}


struct BusReply{
    int id_;
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
            return bus->stop_names_.size();
        }
        return nullopt;
    }

    optional<double> GetRouteLength(const BusCatalog& catalog, const BusInfo* bus){
        if(bus){
            double length = 0;
            for(auto it=bus->stop_names_.begin(); it!=prev(bus->stop_names_.end()); it++){
                auto lhs = it;
                auto rhs = next(it);
                length +=catalog.ComputeDistance(*lhs, *rhs);
            }
            if(bus->type_==BusInfo::Type::STRAIGHT){
                return 2*length;
            }
            else{
                length+= catalog.ComputeDistance(bus->stop_names_.back(), bus->stop_names_.front());
                return length;
            }
        }
        return nullopt;
    }

    BusReply(const BusCatalog& catalog, int bus_id): id_(bus_id){
        const BusInfo* bus = catalog.GetBus(bus_id);
        num_stops_ = GetAllStopsNum(bus);
        unique_stops_ = GetUniqueStopsNum(bus);
        route_len_ = GetRouteLength(catalog, bus);
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


void ProcessRequests(const BusCatalog& catalog, const vector<int> bus_ids, ostream& out_stream=cout){
    for(int el : bus_ids){
        BusReply reply(catalog, el);
        reply.Reply(out_stream);
    }
}



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
    ASSERT_EQUAL(750, ConvertToInt(tmp));
    ASSERT_EQUAL(BusInfo::Type::STRAIGHT, GetBusType(test));
    ASSERT_EQUAL(BusInfo::Type::ROUND, GetBusType("test > test > test"));
    ASSERT_EQUAL("N1", GetUntilSplitter(test, '-'));
    ASSERT_EQUAL("Stop 2", GetUntilSplitter(test, '-'));
    ASSERT_EQUAL("Stop number 3", GetUntilSplitter(test, '-'));
}


void TestReadInput(){
    BusCatalog bc;
    {
        stringstream ss;
        ss << 3 << "\n";
        ss << "Stop Stop N1: 3.14, 6.28\n";
        ss << "Bus 72: Zyzka - Kaluzka - Tepliy stan\n";
        ss << "Bus 256: Stop N1 > StopN2 > S T O P N 3 > Stop N1\n";
        ReadInputData(bc, ss);
    }
    const auto stops = bc.GetStops("Stop N1");
    ASSERT_EQUAL("Stop N1", stops->name_);
    ASSERT_EQUAL(3.14, stops->latitude_);
    ASSERT_EQUAL(6.28, stops->longitude_);
    const auto bus1 = bc.GetBus(72);
    ASSERT_EQUAL(bus1->id_, 72);
    ASSERT_EQUAL(bus1->type_, BusInfo::Type::STRAIGHT);
    list<string_view> expected1 = {"Zyzka", "Kaluzka", "Tepliy stan"};
    ASSERT_EQUAL(bus1->stop_names_, expected1);
    const auto bus2 = bc.GetBus(256);
    ASSERT_EQUAL(bus2->id_, 256);
    ASSERT_EQUAL(bus2->type_, BusInfo::Type::ROUND);
    list<string_view> expected2 = { "Stop N1", "StopN2", "S T O P N 3"};
    ASSERT_EQUAL(bus2->stop_names_, expected2);

}

void TestReadRequests(){
    stringstream ss;
    ss << 3 << "\n";
    ss << "Bus 72\n" << "Bus 81\n" << "Bus 60\n";
    vector<int> get = ReadRequests(ss);
    vector<int> expected = {72, 81, 60};
    ASSERT_EQUAL(get, expected);
}

int main(){
    TestRunner tr;
    RUN_TEST(tr, TestParcingRequest);
    RUN_TEST(tr, TestReadInput);
    RUN_TEST(tr, TestReadRequests);


    return 0;
}
