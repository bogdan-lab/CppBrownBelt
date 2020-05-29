#include <unordered_map>
#include <optional>
#include <unordered_set>
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <list>
#include <set>
#include <iostream>
#include <cmath>
#include <iostream>


#include "bus_catalog.h"
#include "utils.h"
#include "requests.h"

using namespace std;


ostream& operator<<(ostream& out, const RealDistance& el){
    out << el.first << "; " << el.second;
    return out;
}


Stop::Stop(string_view n, double lat, double lon, vector<RealDistance> dist):
    name_(move(n)), latitude_(move(lat)), longitude_(move(lon)), distances_(move(dist)){}



BusInfo::BusInfo(string_view id, Type type, list<string_view> snames):
    id_(id), type_(type), stop_names_(move(snames)) {}



size_t PairHasher::operator()(const pair<string_view, string_view>& el) const {
    return  2946901*sv_hash(el.first) + sv_hash(el.second);
}


bool operator==(const pair<string_view, string_view>& lhs, const pair<string_view, string_view>& rhs){
    return lhs.first==rhs.first && lhs.second==rhs.second;
}



string_view BusCatalog::SaveStopName(string_view new_stop_name){
    auto res = stop_names_.insert(string(new_stop_name));
    return *(res.first);
}

string_view BusCatalog::SaveBusName(string_view new_bus_name){
    auto res = bus_names_.insert(string(new_bus_name));
    return *(res.first);
}

double BusCatalog::ComputeDistance(string_view lhs, string_view rhs) const {
    const Stop *left = stops_.at(lhs).get();
    const Stop *right = stops_.at(rhs).get();
    double cos_alpha = sin(left->latitude_)*sin(right->latitude_) +
            cos(left->latitude_)*cos(right->latitude_) *
            cos(abs(left->longitude_ - right->longitude_));
    return R_EARTH*acos(cos_alpha);     //in m
}

double BusCatalog::ComputeBusRouteLen(const BusInfo* bus) const {
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

size_t BusCatalog::ComputeBusRealDist(const BusInfo* bus) const {
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

unordered_set<string_view> BusCatalog::GetUniqueStops(const BusInfo* bus) const {
    unordered_set<string_view> unique;
    for(const auto& el : bus->stop_names_){
        unique.insert(el);
    }
    return unique;
}

void BusCatalog::UpdateBusRouteLens(){
    for(const auto& name : bus_names_){
        buses_[name]->route_len_ = ComputeBusRouteLen(buses_[name].get());
        buses_[name]->real_dist_ = ComputeBusRealDist(buses_[name].get());
    }
}

void BusCatalog::UpdateBusUniqueStops(){
    for(const auto& name : bus_names_){
        buses_[name]->unique_stops_ = GetUniqueStops(buses_[name].get());
    }
}

void BusCatalog::UpdateStopToBus(){
    for(const auto& bus_pair : buses_){
        const auto *bus_ptr = bus_pair.second.get();
        for(const auto& stop_name : bus_ptr->unique_stops_){
            stop_to_bus_[stop_name].insert(bus_ptr->id_);
        }
    }
}

void BusCatalog::AddRealDistance(string_view start, RealDistance& new_dist){
    new_dist.first = SaveStopName(new_dist.first);
    auto fact = make_pair(start, new_dist.first);
    all_distances_[fact] = new_dist.second;
    auto alternative = make_pair(new_dist.first, start);
    all_distances_.insert(make_pair(alternative, new_dist.second));
}

void BusCatalog::AddBusStop(unique_ptr<Stop> new_stop){
    new_stop->name_ = SaveStopName(new_stop->name_);
    new_stop->latitude_ *= PI/180.0;
    new_stop->longitude_ *= PI/180.0;
    for(auto& el : new_stop->distances_){
        AddRealDistance(new_stop->name_, el);
    }
    stops_[new_stop->name_] = move(new_stop);
}

void BusCatalog::AddBus(unique_ptr<BusInfo> new_bus){
    new_bus->id_ = SaveBusName(new_bus->id_);
    for(auto& el : new_bus->stop_names_){
        el = SaveStopName(el);
    }
    buses_[new_bus->id_] = move(new_bus);
}

void BusCatalog::UpdateDataBase(){
    UpdateBusRouteLens();
    UpdateBusUniqueStops();
    UpdateStopToBus();
}


optional<set<string_view>> BusCatalog::GetBusesForStop(string_view stop_key) const {
    auto it = stop_to_bus_.find(stop_key);
    if(it!=stop_to_bus_.end()){
        return it->second;
    }
    return nullopt;
}

const Stop* BusCatalog::GetStop(string_view stop_key) const {
    auto it = stops_.find(stop_key);
    if (it!=stops_.end()){
        return it->second.get();
    }
    return nullptr;
}

const BusInfo* BusCatalog::GetBus(string_view bus_id) const {
    auto it = buses_.find(bus_id);
    if (it!=buses_.end()){
        return it->second.get();
    }
    return nullptr;
}

size_t BusCatalog::GetDistStructSize() const {
    return all_distances_.size();
}

void ReadInputData(BusCatalog& catalog, istream& in_stream){
    using namespace Utils;
    using namespace Request;
    size_t request_count = ReadRequestCount(in_stream);
    for (size_t i=0; i< request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view input_name = GetUntilSplitter(request_view, " ");
        InputRequest::Type input_type = STR_TO_INPUT_TYPE.at(input_name);
        InputRequestHolder input = InputRequest::Create(input_type);
        input->ParseFromString(request_view);
        input->Process(catalog);
    }
    catalog.UpdateDataBase();
}

void ProcessRequests(const BusCatalog& catalog, istream& in_stream, ostream& out_stream){
    using namespace Utils;
    using namespace Request;
    size_t request_count = ReadRequestCount(in_stream);
    for(size_t i=0; i<request_count; i++){
        string request_line;
        getline(in_stream, request_line);
        string_view request_view = string_view(request_line);
        string_view request_name = GetUntilSplitter(request_view, " ");
        ReplyRequest::Type type = STR_TO_REQUEST_TYPE.at(request_name);
        ReplyRequestHolder res = ReplyRequest::Create(type);
        res->ParseFromString(catalog, request_view);
        res->Reply(out_stream);
    }
}


