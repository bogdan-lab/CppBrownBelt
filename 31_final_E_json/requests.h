#ifndef REQUESTS_H
#define REQUESTS_H

#include <string_view>
#include <list>
#include <string>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <vector>
#include <unordered_map>

#include "bus_catalog.h"
#include "json.h"


namespace Request {

struct InputRequest;
struct ReplyRequest;
using InputRequestHolder = std::unique_ptr<InputRequest>;
using ReplyRequestHolder = std::unique_ptr<ReplyRequest>;


struct InputRequest{
    enum Type{
        BUS,
        STOP
    };
    InputRequest(Type type);
    virtual ~InputRequest() = default;
    static InputRequestHolder Create(Type type);
    virtual void ParseFrom(std::string_view s) = 0;
    virtual void ParseFrom(const std::map<std::string, Json::Node>& node) = 0;
    virtual void Process(BusCatalog& catalog) const = 0;
    Type type_;
};


const std::unordered_map<std::string_view, InputRequest::Type> STR_TO_INPUT_TYPE = {
    {"Bus", InputRequest::Type::BUS},
    {"Stop", InputRequest::Type::STOP}
};

struct InputStop : InputRequest{
    const size_t NUM_OF_STOPS_ = 100;
    std::string_view stop_name_;
    double latitude_;
    double longitude_;
    std::vector<RealDistance> distances_;

    InputStop();
    void ParseFrom(std::string_view request) override;
    virtual void ParseFrom(const std::map<std::string, Json::Node>& node) override;
    void Process(BusCatalog& catalog) const override;
};

struct InputBus : InputRequest{
    const std::unordered_map<BusInfo::Type, std::string_view> SPLITTER_FOR = {{BusInfo::Type::ROUND   , ">"},
                                                                    {BusInfo::Type::STRAIGHT, "-"}};

    std::string_view id_;
    std::list<std::string_view> stop_names_;
    BusInfo::Type type_;

    InputBus();
    BusInfo::Type GetBusType(std::string_view s);
    std::list<std::string_view> ReadBusStops(std::string_view request, std::string_view splitter);
    void ParseFrom(std::string_view request) override ;
    virtual void ParseFrom(const std::map<std::string, Json::Node>& node) override;
    void Process(BusCatalog& catalog) const override ;
};


struct ReplyRequest{
    enum Type{
        BUS,
        STOP
    };
    Type type_;

    ReplyRequest(Type type);
    virtual ~ReplyRequest()=default;
    ReplyRequest(const ReplyRequest& other) = delete;
    ReplyRequest& operator=(const ReplyRequest& other) = delete;
    static ReplyRequestHolder Create(Type type);
    virtual void ParseFrom(const BusCatalog& catalog, std::string_view request) = 0;
    virtual void ParseFrom(const BusCatalog& catalog, const std::map<std::string, Json::Node>& request) = 0;
    virtual void Reply(std::ostream& out_stream) const = 0;
    virtual Json::Node Reply() const = 0;
};


const std::unordered_map<std::string_view, ReplyRequest::Type> STR_TO_REQUEST_TYPE = {
    {"Bus", ReplyRequest::Type::BUS},
    {"Stop", ReplyRequest::Type::STOP}
};


struct BusReply : ReplyRequest{
    std::string_view name_ = {};
    int id_ = 0;
    bool found_ = false;
    std::optional<int> num_stops_;
    std::optional<int> unique_stops_;
    std::optional<double> route_len_;
    std::optional<int> real_distance_;

    BusReply();
    std::optional<int> GetAllStopsNum(const BusInfo* bus);
    std::optional<int> GetUniqueStopsNum(const BusInfo* bus);
    std::optional<double> GetRouteLength(const BusInfo* bus);
    std::optional<int> GetRealDistance(const BusInfo* bus);
    void ParseFrom(const BusCatalog& catalog, std::string_view request) override;
    void ParseFrom(const BusCatalog& catalog, const std::map<std::string, Json::Node>& request) override;
    void Reply(std::ostream& out_stream) const override;
    Json::Node Reply() const override;
};

struct StopReply : ReplyRequest{
    bool found_ = false;
    int id_;
    std::string_view stop_name_;
    std::optional<std::set<std::string_view>> buses_;

    StopReply();

    void ParseFrom(const BusCatalog& catalog, std::string_view request) override;
    void ParseFrom(const BusCatalog& catalog, const std::map<std::string, Json::Node>& request) override;
    void Reply(std::ostream& out_stream) const override;
    Json::Node Reply() const override;
};


}



#endif // REQUESTS_H