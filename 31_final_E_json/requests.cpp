#include <string_view>
#include <list>
#include <string>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

#include "requests.h"
#include "bus_catalog.h"
#include "utils.h"
#include "json.h"

using namespace std;


namespace Request{
using namespace Utils;
using namespace Json;

InputRequest::InputRequest(Type type): type_(type) {}

InputStop::InputStop(): InputRequest::InputRequest(InputRequest::Type::STOP) {}

void InputStop::ParseFrom(string_view request) {
    stop_name_ = GetUntilSplitter(request, ":");
    latitude_ = ConvertToDouble(GetUntilSplitter(request, ","));
    longitude_ = ConvertToDouble(GetUntilSplitter(request, ","));
    distances_.reserve(NUM_OF_STOPS_);
    while(!request.empty()){
        size_t dist = ConvertToUInt(GetUntilSplitter(request, "m to"));
        string_view other_stop = GetUntilSplitter(request, ",");
        distances_.push_back(make_pair(other_stop, dist));
    }
}

void InputStop::ParseFrom(const map<string, Node>& request){
    stop_name_ = request.at("name").AsString();
    latitude_ = request.at("latitude").AsDouble();
    longitude_ = request.at("longitude").AsDouble();
    map<string, Node> given_distances = request.at("road_distances").AsMap();
    distances_.reserve(given_distances.size());
    for(const auto& el : given_distances){
        string_view station = el.first;
        size_t dist = el.second.DoubleAsInt();
        distances_.push_back(make_pair(station, dist));
    }

}

void InputStop::Process(BusCatalog& catalog) const {
    catalog.AddBusStop(make_unique<Stop>(Stop(stop_name_, latitude_, longitude_, distances_)));
}


InputBus::InputBus(): InputRequest::InputRequest(InputRequest::Type::BUS) {}

BusInfo::Type InputBus::GetBusType(string_view s){
    string_view check_str = s.substr(0, 28);
    size_t status = check_str.find('-');
    if(status!=string::npos){
        return BusInfo::Type::STRAIGHT;
    }
    return BusInfo::Type::ROUND;
}

list<string_view> InputBus::ReadBusStops(string_view request, string_view splitter){
    list<string_view> stop_names;
    while(!request.empty()){
        stop_names.push_back(GetUntilSplitter(request, splitter));
    }
    return stop_names;
}

void InputBus::ParseFrom(string_view request) {
    id_ = GetUntilSplitter(request, ":");
    type_ = GetBusType(request);
    stop_names_ = ReadBusStops(request, SPLITTER_FOR.at(type_));
    if(type_==BusInfo::Type::ROUND){stop_names_.pop_back();}
}

void InputBus::ParseFrom(const map<string, Node>& request) {
    id_ = request.at("name").AsString();
    type_ = request.at("is_roundtrip").AsBool() ? BusInfo::Type::ROUND : BusInfo::Type::STRAIGHT ;
    vector<Node> given_stops = request.at("stops").AsArray();
    for(const auto& el : given_stops){
        stop_names_.push_back(el.AsString());
    }
    if(type_==BusInfo::Type::ROUND){stop_names_.pop_back();}
}

void InputBus::Process(BusCatalog& catalog) const {
    catalog.AddBus(make_unique<BusInfo>(BusInfo(id_, type_, move(stop_names_))));
}


InputRequestHolder InputRequest::Create(Type type){
    switch (type) {
    case InputRequest::Type::STOP:
        return make_unique<InputStop>();
    case InputRequest::Type::BUS:
        return make_unique<InputBus>();
    }
    return nullptr;
}





    ReplyRequest::ReplyRequest(Type type) : type_(type) {}


    BusReply::BusReply(): ReplyRequest::ReplyRequest(ReplyRequest::Type::BUS) {}

    optional<int> BusReply::GetAllStopsNum(const BusInfo* bus){
        if(bus){
            if(bus->type_==BusInfo::Type::STRAIGHT){return 2*bus->stop_names_.size()-1;}
            else {return bus->stop_names_.size()+1;}
        }
        return nullopt;
    }

    optional<int> BusReply::GetUniqueStopsNum(const BusInfo* bus){
        if(bus){
            return bus->unique_stops_.size();
        }
        return nullopt;
    }

    optional<double> BusReply::GetRouteLength(const BusInfo* bus){
        if(bus){
            return bus->route_len_;
        }
        return nullopt;
    }

    optional<int> BusReply::GetRealDistance(const BusInfo* bus){
        if (bus){
            return bus->real_dist_;
        }
        return nullopt;
    }

    void BusReply::ParseFrom(const BusCatalog& catalog, string_view request) {
        name_ = DeleteSpaces(request);
        const BusInfo* bus = catalog.GetBus(name_);
        found_ = bus;
        num_stops_ = GetAllStopsNum(bus);
        unique_stops_ = GetUniqueStopsNum(bus);
        route_len_ = GetRouteLength(bus);
        real_distance_ = GetRealDistance(bus);
    }

    void BusReply::ParseFrom(const BusCatalog &catalog, const std::map<string, Node> &request){
        name_ = request.at("name").AsString();
        id_ = request.at("id").DoubleAsInt();
        const BusInfo* bus = catalog.GetBus(name_);
        found_ = bus;
        num_stops_ = GetAllStopsNum(bus);
        unique_stops_ = GetUniqueStopsNum(bus);
        route_len_ = GetRouteLength(bus);
        real_distance_ = GetRealDistance(bus);
    }

    void BusReply::Reply(ostream& out_stream) const {
        if(found_){
            out_stream << "Bus " << name_ <<": "<< num_stops_.value() << " stops on route, "
                       << unique_stops_.value() << " unique stops, " <<real_distance_.value() << " route length, "
                       << real_distance_.value()/route_len_.value() << " curvature\n";
        } else {
            out_stream << "Bus " << name_ <<": not found\n";
        }
    }

    Node BusReply::Reply() const{
        map<string, Node> reply;
        reply["request_id"] = Node(id_);
        if(found_){
            reply["route length"] = Node(real_distance_.value());
            reply["curvature"] = Node(real_distance_.value()/route_len_.value());
            reply["stop_count"] = Node(num_stops_.value());
            reply["unique_stop_count"] = Node(unique_stops_.value());
        }
        else{
            reply["error_message"] = Node("not found");
        }
        return Node(reply);
    }

   StopReply::StopReply(): ReplyRequest::ReplyRequest(ReplyRequest::Type::STOP) {}

    void StopReply::ParseFrom(const BusCatalog& catalog, string_view request) {
        stop_name_ = DeleteSpaces(request);
        const auto* stop = catalog.GetStop(stop_name_);
        found_ = stop;
        buses_ = catalog.GetBusesForStop(stop_name_);
    }

    void StopReply::ParseFrom(const BusCatalog &catalog, const std::map<string, Node> &request){
        stop_name_ = request.at("name").AsString();
        id_ = request.at("id").DoubleAsInt();
        const auto* stop = catalog.GetStop(stop_name_);
        found_ = stop;
        buses_ = catalog.GetBusesForStop(stop_name_);
    }

    void StopReply::Reply(ostream& out_stream) const {
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

    Node StopReply::Reply() const{
        map<string, Node> reply;
        reply["request_id"] = Node(id_);
        if (found_){
            vector<Node> buses_to_stop;
            if(buses_){
                buses_to_stop.reserve(buses_.value().size());
                for(const auto& el : buses_.value()){
                    buses_to_stop.push_back(Node(string(el)));
                }
            }
            reply["buses"] = Node(buses_to_stop);
        }
        else {
            reply["error_message"] = Node("not found");
        }
        return Node(reply);
    }

    ReplyRequestHolder ReplyRequest::Create(ReplyRequest::Type type){
    switch (type) {
    case ReplyRequest::Type::BUS:
        return make_unique<BusReply>();
    case ReplyRequest::Type::STOP:
        return make_unique<StopReply>();
    }
    return nullptr;
}



}
