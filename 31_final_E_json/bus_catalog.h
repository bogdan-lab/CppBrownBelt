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

using RealDistance = std::pair<std::string_view, size_t>;

std::ostream& operator<<(std::ostream& out, const RealDistance& el);

struct Stop{
    std::string_view name_;
    double latitude_;
    double longitude_;
    std::vector<RealDistance> distances_ = {};

    Stop(std::string_view n, double lat, double lon, std::vector<RealDistance> dist);
};



struct BusInfo{
    enum Type{
        ROUND,
        STRAIGHT,
    };

    std::string_view id_;
    Type type_;
    std::list<std::string_view> stop_names_;
    std::unordered_set<std::string_view> unique_stops_ = {};
    double route_len_ = 0.0;
    size_t real_dist_ = 0;

    BusInfo(std::string_view id, Type type, std::list<std::string_view> snames);
};



struct PairHasher{
    std::hash<std::string_view> sv_hash;
    size_t operator()(const std::pair<std::string_view, std::string_view>& el) const;
};

bool operator==(const std::pair<std::string_view, std::string_view>& lhs, const std::pair<std::string_view, std::string_view>& rhs);



class BusCatalog{
    using StopStruct = std::unordered_map<std::string_view, std::unique_ptr<Stop>>;
    using BusStruct = std::unordered_map<std::string_view, std::unique_ptr<BusInfo>>;
    using DistStruct = std::unordered_map<std::pair<std::string_view, std::string_view>, size_t, PairHasher>;


private:
    const double PI = 3.1415926535;
    const double R_EARTH = 6371000;     //m
    std::unordered_set<std::string> bus_names_;
    std::unordered_set<std::string> stop_names_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_bus_;
    StopStruct stops_;
    BusStruct buses_;
    DistStruct all_distances_;


    std::string_view SaveStopName(std::string_view new_stop_name);
    std::string_view SaveBusName(std::string_view new_bus_name);
    double ComputeDistance(std::string_view lhs, std::string_view rhs) const;
    double ComputeBusRouteLen(const BusInfo* bus) const;
    size_t ComputeBusRealDist(const BusInfo* bus) const;
    std::unordered_set<std::string_view> GetUniqueStops(const BusInfo* bus) const;
    void UpdateBusRouteLens();
    void UpdateBusUniqueStops();
    void UpdateStopToBus();
    void AddRealDistance(std::string_view start, RealDistance& new_dist);

public:

    void AddBusStop(std::unique_ptr<Stop> new_stop);
    void AddBus(std::unique_ptr<BusInfo> new_bus);
    void UpdateDataBase();
    std::optional<std::set<std::string_view>> GetBusesForStop(std::string_view stop_key) const ;
    const Stop* GetStop(std::string_view stop_key) const ;
    const BusInfo* GetBus(std::string_view bus_id) const ;
    size_t GetDistStructSize() const ;

};

void ReadInputData(BusCatalog& catalog, std::istream& in_stream=std::cin);
void ProcessRequests(const BusCatalog& catalog, std::istream& in_stream=std::cin, std::ostream& out_stream=std::cout);

#endif
