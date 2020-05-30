#ifndef BUS_CATALOG
#define BUS_CATALOG

#include <unordered_map>
#include <optional>
#include <unordered_set>
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <list>
#include <set>
#include <vector>
#include <iostream>

#include "json.h"

using RealDistance = std::pair<std::string, int>;

std::ostream& operator<<(std::ostream& out, const RealDistance& el);

struct Stop{
    std::string name_;
    double latitude_;
    double longitude_;
    std::vector<RealDistance> distances_ = {};

    Stop(std::string n, double lat, double lon, std::vector<RealDistance> dist);
};



struct BusInfo{
    enum Type{
        ROUND,
        STRAIGHT,
    };

    std::string id_;
    Type type_;
    std::vector<std::string> stop_names_;
    std::unordered_set<std::string> unique_stops_ = {};
    double route_len_ = 0.0;
    int real_dist_ = 0;

    BusInfo(std::string id, Type type, std::vector<std::string> snames);
};



struct PairHasher{
    std::hash<std::string> sv_hash;
    size_t operator()(const std::pair<std::string, std::string>& el) const;
};

bool operator==(const std::pair<std::string, std::string>& lhs, const std::pair<std::string, std::string>& rhs);



class BusCatalog{
    using StopStruct = std::unordered_map<std::string, std::unique_ptr<Stop>>;
    using BusStruct = std::unordered_map<std::string, std::unique_ptr<BusInfo>>;
    using DistStruct = std::unordered_map<std::pair<std::string, std::string>, int, PairHasher>;


private:
    const double PI = 3.1415926535;
    const double R_EARTH = 6371000;     //m
    std::unordered_set<std::string> bus_names_;
    std::unordered_set<std::string> stop_names_;
    std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
    StopStruct stops_;
    BusStruct buses_;
    DistStruct all_distances_;

    std::string SaveStopName(std::string new_stop_name);
    std::string SaveBusName(std::string new_bus_name);
    double ComputeDistance(std::string lhs, std::string rhs) const;
    double ComputeBusRouteLen(const BusInfo* bus) const;
    int ComputeBusRealDist(const BusInfo* bus) const;
    std::unordered_set<std::string> GetUniqueStops(const BusInfo* bus) const;
    void UpdateBusRouteLens();
    void UpdateBusUniqueStops();
    void UpdateStopToBus();
    void AddRealDistance(std::string start, RealDistance& new_dist);

public:

    void AddBusStop(std::unique_ptr<Stop> new_stop);
    void AddBus(std::unique_ptr<BusInfo> new_bus);
    void UpdateDataBase();
    std::set<std::string> GetBusesForStop(std::string stop_key) const ;
    const Stop* GetStop(std::string stop_key) const ;
    const BusInfo* GetBus(std::string bus_id) const ;
    int GetDistStructSize() const ;

};

void ReadInputData(BusCatalog& catalog, const std::vector<Json::Node>& base_requests);
void ProcessRequests(const BusCatalog& catalog, const std::vector<Json::Node>& stat_requests, std::ostream& out_stream=std::cout);

#endif
