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

//#include "test_runner.h"


using namespace std;



string_view DeleteSpaces(string_view s){
    while(s.back()==' '){s.remove_suffix(1);}
    while(s.front()==' '){s.remove_prefix(1);}
    return s;
}

string_view GetUntilSplitter(string_view& s, string_view splitter){
    size_t pos = s.find(splitter.front());
    string_view info;
    if (pos != string::npos){
        info = s.substr(0, pos);
        s.remove_prefix(pos+splitter.size());
    }else {s.swap(info);}
    return DeleteSpaces(info);
}

double ConvertToDouble(string_view s){
    string_view tmp = DeleteSpaces(s);
    return stod(string(tmp), nullptr);
}


size_t ConvertToUInt(string_view s){
    string_view tmp = DeleteSpaces(s);
    return stoul(string(tmp), nullptr);
}

using RealDistance = pair<string_view, size_t>;

ostream& operator<<(ostream& out, const RealDistance& el){
    out << el.first << "; " << el.second;
    return out;
}

struct Stop{
    string_view name_;
    double latitude_;
    double longitude_;
    vector<RealDistance> distances_ = {};

    Stop(string_view n, double lat, double lon, vector<RealDistance> dist):
        name_(move(n)), latitude_(move(lat)), longitude_(move(lon)), distances_(move(dist)){}
};



struct BusInfo{
    enum Type{
        ROUND,
        STRAIGHT,
    };

    string_view id_;
    Type type_;
    list<string_view> stop_names_;
    unordered_set<string_view> unique_stops_ = {};
    double route_len_ = 0.0;
    size_t real_dist_ = 0.0;

    BusInfo(string_view id, Type type, list<string_view> snames):
        id_(id), type_(type), stop_names_(move(snames)) {}
};


struct PairHasher{
    hash<string_view> sv_hash;

    size_t operator()(const pair<string_view, string_view>& el) const {
        return  2946901*sv_hash(el.first) + sv_hash(el.second);
    }

};

bool operator==(const pair<string_view, string_view>& lhs, const pair<string_view, string_view>& rhs){
    return lhs.first==rhs.first && lhs.second==rhs.second;
}

class BusCatalog{
using StopStruct = unordered_map<string_view, unique_ptr<Stop>>;
using BusStruct = unordered_map<string_view, unique_ptr<BusInfo>>;


using DistStruct = unordered_map<pair<string_view, string_view>, size_t, PairHasher>;

private:

    const double PI = 3.1415926535;
    const double R_EARTH = 6371000;     //m

    unordered_set<string> bus_names_;
    unordered_set<string> stop_names_;
    unordered_map<string_view, set<string_view>> stop_to_bus_;
    StopStruct stops_;
    BusStruct buses_;
    DistStruct all_distances_;


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
//        double cos_alpha = sin(left->latitude_)*sin(right->latitude_) +
//                cos(left->latitude_)*cos(right->latitude_)*(sin(left->longitude_)*sin(right->longitude_) +
//                                                          cos(left->longitude_)*cos(right->longitude_));
        double cos_alpha = sin(left->latitude_)*sin(right->latitude_) +
                           cos(left->latitude_)*cos(right->latitude_) *
                           cos(abs(left->longitude_ - right->longitude_));
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
            length*=2;
        }
        else{
            length+= this->ComputeDistance(bus->stop_names_.back(), bus->stop_names_.front());
        }
        return length;
    }

    size_t ComputeBusRealDist(const BusInfo* bus) const {
        size_t length = 0;
        for(auto it=bus->stop_names_.begin(); it!=prev(bus->stop_names_.end()); it++){
            auto start = it;
            auto end = next(it);
            auto interval = make_pair(*start, *end);
            length +=all_distances_.at(interval);
        }
        if(bus->type_==BusInfo::Type::STRAIGHT){
            //go backward
            for(auto it = prev(bus->stop_names_.end()); it!=next(bus->stop_names_.end()); it--){
                auto start = it;
                auto end = prev(it);
                auto interval = make_pair(*start, *end);
                length += all_distances_.at(interval);
            }
        }
        else {
            auto final_interval = make_pair(bus->stop_names_.back(), bus->stop_names_.front());
            length+=all_distances_.at(final_interval);
        }
        return length;
    }

    unordered_set<string_view> GetUniqueStops(const BusInfo* bus) const {
        unordered_set<string_view> unique;
        for(const auto& el : bus->stop_names_){
            unique.insert(el);
        }
        return unique;
    }

    void UpdateBusRouteLens(){
        for(const auto& name : bus_names_){
            buses_[name]->route_len_ = ComputeBusRouteLen(buses_[name].get());
            buses_[name]->real_dist_ = ComputeBusRealDist(buses_[name].get());
        }
    }

    void UpdateBusUniqueStops(){
        for(const auto& name : bus_names_){
            buses_[name]->unique_stops_ = GetUniqueStops(buses_[name].get());
        }
    }

    void UpdateStopToBus(){
        for(const auto& bus_pair : buses_){
            const auto *bus_ptr = bus_pair.second.get();
            for(const auto& stop_name : bus_ptr->unique_stops_){
                stop_to_bus_[stop_name].insert(bus_ptr->id_);
            }
        }
    }

    void AddRealDistance(string_view start, RealDistance& new_dist){
        new_dist.first = SaveStopName(new_dist.first);
        auto fact = make_pair(start, new_dist.first);
        all_distances_[fact] = new_dist.second;
        auto alternative = make_pair(new_dist.first, start);
        all_distances_.insert(make_pair(alternative, new_dist.second));
    }

public:
    void AddBusStop(unique_ptr<Stop> new_stop){
        new_stop->name_ = SaveStopName(new_stop->name_);
        new_stop->latitude_ *= PI/180.0;
        new_stop->longitude_ *= PI/180.0;
        for(auto& el : new_stop->distances_){
            AddRealDistance(new_stop->name_, el);
        }
        stops_[new_stop->name_] = move(new_stop);
    }

    void AddBus(unique_ptr<BusInfo> new_bus){
        new_bus->id_ = SaveBusName(new_bus->id_);
        for(auto& el : new_bus->stop_names_){
            el = SaveStopName(el);
        }
        buses_[new_bus->id_] = move(new_bus);
    }

    void UpdateDataBase(){
        UpdateBusRouteLens();
        UpdateBusUniqueStops();
        UpdateStopToBus();
    }


    optional<set<string_view>> GetBusesForStop(string_view stop_key) const {
        auto it = stop_to_bus_.find(stop_key);
        if(it!=stop_to_bus_.end()){
            return it->second;
        }
        return nullopt;
    }

    const Stop* GetStop(string_view stop_key) const {
        auto it = stops_.find(stop_key);
        if (it!=stops_.end()){
            return it->second.get();
        }
        return nullptr;
    }

    const BusInfo* GetBus(string_view bus_id) const {
        auto it = buses_.find(bus_id);
        if (it!=buses_.end()){
            return it->second.get();
        }
        return nullptr;
    }

    size_t GetDistStructSize() const {
        return all_distances_.size();
    }
};

unique_ptr<Stop> ReadStop(string_view request){
    const size_t NUM_OF_STOPS = 100;
    string_view stop_name = GetUntilSplitter(request, ":");
    double latitude = ConvertToDouble(GetUntilSplitter(request, ","));
    double longitude = ConvertToDouble(GetUntilSplitter(request, ","));
    vector<RealDistance> distances;
    distances.reserve(NUM_OF_STOPS);
    while(!request.empty()){
        size_t dist = ConvertToUInt(GetUntilSplitter(request, "m to"));
        string_view other_stop = GetUntilSplitter(request, ",");
        distances.push_back(make_pair(other_stop, dist));
    }
    return make_unique<Stop>(Stop(stop_name, latitude, longitude, distances));
}

BusInfo::Type GetBusType(string_view s){
    string_view check_str = s.substr(0, 28);
    size_t status = check_str.find('-');
    if(status!=string::npos){
        return BusInfo::Type::STRAIGHT;
    }
    return BusInfo::Type::ROUND;
}

string_view GetSplitter(BusInfo::Type type){
    if (type==BusInfo::ROUND){
        return ">";
    }
    return "-";
}

list<string_view> ReadBusStops(string_view request, string_view splitter){
    list<string_view> stop_names;
    while(!request.empty()){
        stop_names.push_back(GetUntilSplitter(request, splitter));
    }
    return stop_names;
}

unique_ptr<BusInfo> ReadBus(string_view request){
    string_view id = GetUntilSplitter(request, ":");
    BusInfo::Type type = GetBusType(request);
    string_view splitter = GetSplitter(type);
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
        string_view data_name = GetUntilSplitter(request_view, " ");
        if(data_name=="Bus"){
            unique_ptr<BusInfo> bus = ReadBus(request_view);
            catalog.AddBus(move(bus));
        } else {
            unique_ptr<Stop> stop = ReadStop(request_view);
            catalog.AddBusStop(move(stop));
        }
    }
    catalog.UpdateDataBase();
}


struct Request;
using RequestHolder = unique_ptr<Request>;


struct Request{

    enum Type{
        BUS,
        STOP
    };

Type type_;

Request(Type type) : type_(type) {}
virtual ~Request()=default;
Request(const Request& other) = delete;
Request& operator=(const Request& other) = delete;

static RequestHolder Create(Type type);
virtual void ParseFromString(const BusCatalog& catalog, string_view request) = 0;
virtual void Reply(ostream& out_stream) const = 0;
};


const unordered_map<string_view, Request::Type> STR_TO_REQUEST_TYPE = {
    {"Bus", Request::Type::BUS},
    {"Stop", Request::Type::STOP}
};


struct BusReply : Request{
    string_view id_ = {};
    bool found_ = false;
    optional<size_t> num_stops_;
    optional<size_t> unique_stops_;
    optional<double> route_len_;
    optional<size_t> real_distance_;

    BusReply(): Request::Request(Request::Type::BUS) {}

    optional<size_t> GetAllStopsNum(const BusInfo* bus){
        if(bus){
            if(bus->type_==BusInfo::Type::STRAIGHT){return 2*bus->stop_names_.size()-1;}
            else {return bus->stop_names_.size()+1;}
        }
        return nullopt;
    }

    optional<size_t> GetUniqueStopsNum(const BusInfo* bus){
        if(bus){
            return bus->unique_stops_.size();
        }
        return nullopt;
    }

    optional<double> GetRouteLength(const BusInfo* bus){
        if(bus){
            return bus->route_len_;
        }
        return nullopt;
    }

    optional<size_t> GetRealDistance(const BusInfo* bus){
        if (bus){
            return bus->real_dist_;
        }
        return nullopt;
    }

    void ParseFromString(const BusCatalog& catalog, string_view request) override{
        id_ = DeleteSpaces(request);
        const BusInfo* bus = catalog.GetBus(id_);
        found_ = bus;
        num_stops_ = GetAllStopsNum(bus);
        unique_stops_ = GetUniqueStopsNum(bus);
        route_len_ = GetRouteLength(bus);
        real_distance_ = GetRealDistance(bus);
    }

    void Reply(ostream& out_stream) const override{
        if(found_){
            out_stream << "Bus " << id_ <<": "<< num_stops_.value() << " stops on route, "
                       << unique_stops_.value() << " unique stops, " <<real_distance_.value() << " route length, "
                       << real_distance_.value()/route_len_.value() << " curvature\n";
        } else {
            out_stream << "Bus " << id_ <<": not found\n";
        }
    }
};


struct StopReply : Request{
    bool found_ = false;
    string_view stop_name_;
    optional<set<string_view>> buses_;

    StopReply(): Request::Request(Request::Type::STOP) {}

    void ParseFromString(const BusCatalog& catalog, string_view request) override{
        stop_name_ = DeleteSpaces(request);
        const auto* stop = catalog.GetStop(stop_name_);
        found_ = stop;
        buses_ = catalog.GetBusesForStop(stop_name_);
    }

    void Reply(ostream& out_stream) const override{
        if(found_){
            if(buses_){
                out_stream << "Stop " << stop_name_ << ": buses";
                for(const auto& el : buses_.value()){
                    out_stream << " " << el;
                }
                out_stream << "\n";
            }
            else {
                out_stream << "Stop " << stop_name_ <<": no buses\n";
            }
        }
        else {
            out_stream << "Stop " << stop_name_ <<": not found\n";
        }
    }
};






RequestHolder Request::Create(Request::Type type){
    switch (type) {
    case Request::Type::BUS:
        return make_unique<BusReply>();
    case Request::Type::STOP:
        return make_unique<StopReply>();
    default:
        return nullptr;
    }
}

void ProcessRequests(const BusCatalog& catalog, istream& in_stream=cin, ostream& out_stream=cout){
    size_t request_count = ReadRequestCount(in_stream);
    for(size_t i=0; i<request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view request_name = GetUntilSplitter(request_view, " ");
        Request::Type type = STR_TO_REQUEST_TYPE.at(request_name);
        RequestHolder res = Request::Create(type);
        res->ParseFromString(catalog, request_view);
        res->Reply(out_stream);
    }
}



//-------------TESTS------------------------

/*

void TestParcingRequest(){
    string_view test = "Stop Tolstopaltsevo: 55.611087, 37.20829";
    ASSERT_EQUAL("Stop", GetUntilSplitter(test, " "));
    ASSERT_EQUAL("Tolstopaltsevo", GetUntilSplitter(test, ":"));
    string_view tmp = GetUntilSplitter(test, ",");
    ASSERT_EQUAL("55.611087", tmp);
    ASSERT_EQUAL(55.611087, ConvertToDouble(tmp));
    ASSERT_EQUAL(37.20829, ConvertToDouble(test));

    test = "Stop FancyStop: 15.0, 20.0, 150m to N1, 250m to N2, 350m to N3";
    GetUntilSplitter(test, " ");
    auto get = ReadStop(test);
    ASSERT_EQUAL("FancyStop", get->name_);
    ASSERT_EQUAL(15.0, get->latitude_);
    ASSERT_EQUAL(20.0, get->longitude_);
    vector<RealDistance> expected = {{"N1", 150},
                                     {"N2", 250},
                                     {"N3", 350}};
    for(size_t i = 0; i<expected.size(); i++){
        ASSERT_EQUAL(expected[i], get->distances_[i]);
    }

    test = "Bus 750: N1 - Stop 2 - Stop number 3";
    ASSERT_EQUAL("Bus", GetUntilSplitter(test, " "));
    tmp = GetUntilSplitter(test, ":");
    ASSERT_EQUAL("750", tmp);
    ASSERT_EQUAL(BusInfo::Type::STRAIGHT, GetBusType(test));
    ASSERT_EQUAL(BusInfo::Type::ROUND, GetBusType("test > test > test"));
    ASSERT_EQUAL("N1", GetUntilSplitter(test, "-"));
    ASSERT_EQUAL("Stop 2", GetUntilSplitter(test, "-"));
    ASSERT_EQUAL("Stop number 3", GetUntilSplitter(test, "-"));
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
    vector<string> expected = {"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length 0.0 curvature",
                               "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length 0.0 curvature",
                               "Bus 751: not found"};
    for(size_t i=0; i<expected.size(); i++){
        string get;
        getline(out, get);
        ASSERT_EQUAL(get, expected[i]);
    }
}


void TestWithStops(){
    stringstream in;
    in<<13<<"\n";
    in << "Stop Tolstopaltsevo: 55.611087, 37.20829\n";
    in << "Stop Marushkino: 55.595884, 37.209755\n";
    in << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n";
    in << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n";
    in << "Stop Rasskazovka: 55.632761, 37.333324\n";
    in << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n";
    in << "Stop Biryusinka: 55.581065, 37.64839\n";
    in << "Stop Universam: 55.587655, 37.645687\n";
    in << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n";
    in << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";
    in << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n";
    in << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n";
    in << "Stop Prazhskaya: 55.611678, 37.603831\n";
    in << "6\n";
    in << "Bus 256\n";
    in << "Bus 750\n";
    in << "Bus 751\n";
    in << "Stop Samara\n";
    in << "Stop Prazhskaya\n";
    in << "Stop Biryulyovo Zapadnoye\n";
    BusCatalog catalog;
    ReadInputData(catalog, in);
    stringstream out;
    ProcessRequests(catalog, in, out);
    vector<string> expected = {"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length 0.0 curvature",
                                "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length 0.0 curvature",
                                "Bus 751: not found",
                                "Stop Samara: not found",
                                "Stop Prazhskaya: no buses",
                                "Stop Biryulyovo Zapadnoye: buses 256 828"};
    for(size_t i=0; i<expected.size(); i++){
        string get;
        getline(out,get);
        ASSERT_EQUAL(get, expected[i]);
    }

}

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

*/


int main(){
//    TestRunner tr;
//    RUN_TEST(tr, TestParcingRequest);
//    RUN_TEST(tr, TestReadStops);
//    RUN_TEST(tr, TestReadBuses);
//    RUN_TEST(tr, TestBusReply);
//    RUN_TEST(tr, TestWithStops);
//    RUN_TEST(tr, TestSavingExtraStops);
//    RUN_TEST(tr, TestNewFormatStops);


    BusCatalog catalog;
    ReadInputData(catalog);
    cout << setprecision(6);
    ProcessRequests(catalog);

    return 0;
}
