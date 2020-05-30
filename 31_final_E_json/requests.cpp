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
#include "json.h"

using namespace std;


namespace Request{
using namespace Json;

InputRequest::InputRequest(Type type): type_(type) {}

InputStop::InputStop(): InputRequest::InputRequest(InputRequest::Type::STOP) {}


void InputStop::ParseFrom(const map<string, Node>& request){
    stop_name_ = request.at("name").AsString();
    latitude_ = request.at("latitude").AsDouble();
    longitude_ = request.at("longitude").AsDouble();
    map<string, Node> given_distances = request.at("road_distances").AsMap();
    distances_.reserve(given_distances.size());
    for(const auto& el : given_distances){
        string station = el.first;
        size_t dist = el.second.DoubleAsInt();
        distances_.push_back(make_pair(station, dist));
    }

}

void InputStop::Process(BusCatalog& catalog) const {
    catalog.AddBusStop(make_unique<Stop>(Stop(stop_name_, latitude_, longitude_, distances_)));
}


InputBus::InputBus(): InputRequest::InputRequest(InputRequest::Type::BUS) {}

void InputBus::ParseFrom(const map<string, Node>& request) {
    id_ = request.at("name").AsString();
    type_ = request.at("is_roundtrip").AsBool() ? BusInfo::Type::ROUND : BusInfo::Type::STRAIGHT ;
    vector<Node> given_stops = request.at("stops").AsArray();
    stop_names_.reserve(given_stops.size());
    for(const auto& el : given_stops){
        stop_names_.push_back(move(el.AsString()));
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
            reply["error_message"] = Node(string("not found"));
        }
        return Node(reply);
    }

   StopReply::StopReply(): ReplyRequest::ReplyRequest(ReplyRequest::Type::STOP) {}

    void StopReply::ParseFrom(const BusCatalog &catalog, const std::map<string, Node> &request){
        stop_name_ = request.at("name").AsString();
        id_ = request.at("id").DoubleAsInt();
        const auto* stop = catalog.GetStop(stop_name_);
        found_ = stop;
        buses_ = catalog.GetBusesForStop(stop_name_);
    }

    Node StopReply::Reply() const{
        map<string, Node> reply;
        reply["request_id"] = Node(id_);
        if (found_){
            vector<Node> buses_to_stop;
            buses_to_stop.reserve(buses_.size());
            for(const auto& el : buses_){
                buses_to_stop.push_back(Node(string(el)));
            }
            reply["buses"] = Node(buses_to_stop);
        }
        else {
            reply["error_message"] = Node(string("not found"));
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
