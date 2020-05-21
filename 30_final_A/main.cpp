#include <iostream>
#include <unordered_map>
#include <list>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <sstream>
#include "test_runner.h"


using namespace std;


string_view DeleteSpaces(string_view s){
    if (s.back() == ' '){s.remove_suffix(1);}
    if (s.front() == ' '){s.remove_prefix(1);}
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
    size_t pos;
    return stod(string(s), &pos);
}

int ConvertToInt(string_view s){
    size_t pos;
    return stoi(string(s), &pos);
}

struct Stop{
    string name_;
    optional<double> latitude_;
    optional<double> longitude_;

    Stop(string n, optional<double> lat, optional<double> lon):
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
     StopStruct stops_;
     BusStruct buses_;

     unique_ptr<BusInfo> RebindStringView(unique_ptr<BusInfo> new_bus){
         for(auto& it : new_bus->stop_names_){

         }
     }
public:
    string_view AddBusStop(unique_ptr<Stop> new_stop){
        string_view stop_name = string_view(new_stop->name_);
        auto it = stops_.find(stop_name);
        if(it==stops_.end()){
            stops_[stop_name] = move(new_stop);
            return stop_name;
        }
        else {
            if(new_stop->latitude_){
                it->second->latitude_ = new_stop->latitude_;
                it->second->longitude_ = new_stop->longitude_;
            }
            return it->first;
        }
    }

    void AddBus(unique_ptr<BusInfo> new_bus){

    }

    Stop* GetStops(string_view stop_key) const {
        return stops_.at(stop_key).get();
    }

};


unique_ptr<Stop> ReadStop(string_view request){
    string_view stop_name = GetUntilSplitter(request, ':');
    double latitude = ConvertToDouble(GetUntilSplitter(request, ','));
    double longitude = ConvertToDouble(request);
    return make_unique<Stop>(Stop(string(stop_name), latitude, longitude));
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



void ReadInputData(BusCatalog& catalog, istream& in_stream){
    size_t request_count;
    in_stream >> request_count;
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
    stringstream ss;
    ss << 1;
    ss << "Stop Stop N1: 3.14, 6.28";
    ReadInputData(bc, ss);
    const auto stops = bc.GetStops("Stop N1");
    ASSERT_EQUAL("Stop N1", stops->name_);
    ASSERT_EQUAL(3.14, stops->latitude_.value());
    ASSERT_EQUAL(6.28, stops->longitude_.value());
}



/*
enum BusType{
    ROUND,
    STRAIGHT
};

BusType GetBusType(string_view s){
    string_view check_str = s.substr(0, 26);
    size_t status = check_str.find("-");
    if(status!=string::npos){
        return BusType::STRAIGHT;
    }
    return BusType::ROUND;
}

const unordered_map<BusType, char> BUS_TP_TO_SPLITTER = {
    {BusType::STRAIGHT, '-'},
    {BusType::ROUND, '>'}
};

struct InputRequest;
using RequestHolder = unique_ptr<InputRequest>;

struct InputRequest{
    enum Type{
        STOP_READ,
        BUS_READ,
    };

    InputRequest(Type type): type_(type) {}
    static RequestHolder Create(Type type);
    virtual  void ParseLine(string_view input) = 0;
    virtual void Process(BusCatalog& catalog) const = 0;
    virtual ~ InputRequest() = default;

    const Type type_;
};


struct SaveBusRequest : InputRequest {
    SaveBusRequest() : InputRequest::InputRequest(InputRequest::Type::BUS_READ) {}

    void ParseLine(string_view input) override {
        string_view id_str = GetUntilSplitter(input, ':');
        id_ = ConvertToInt(id_str);
        bus_type_ = GetBusType(input);
        char splitter = BUS_TP_TO_SPLITTER.at(bus_type_);
        while(!input.empty()){
            stops_.push_back(string(GetUntilSplitter(input, splitter)));
        }
    }

    void Process(BusCatalog& catalog) const override{

    }

    int id_;
    BusType bus_type_;
    list<string> stops_;
};

struct SaveStopRequest : InputRequest {
    SaveStopRequest() : InputRequest::InputRequest(InputRequest::Type::STOP_READ) {}

    void ParseLine(string_view input) override {
        name_ = string(GetUntilSplitter(input, ':'));
        string_view latitude_str = GetUntilSplitter(input, ',');
        latitude_ = ConvertToDouble(latitude_str);
        longitude_ = ConvertToDouble(input);
    }

    void Process(BusCatalog& catalog) const override {

    }

    string name_;
    double latitude_;
    double longitude_;
};

RequestHolder InputRequest::Create(Type type){
    switch (type) {
    case InputRequest::Type::BUS_READ:
        return make_unique<SaveBusRequest>();
    case InputRequest::Type::STOP_READ:
        return make_unique<SaveStopRequest>();
    default:
        return nullptr;
    }
}

const unordered_map<string_view, InputRequest::Type> STR_TO_INPUT_REQUEST = {
    {"Bus", InputRequest::Type::BUS_READ},
    {"Stop", InputRequest::Type::STOP_READ}
};

optional<InputRequest::Type> GetInputRequestType(string_view& request_str){
    string_view request_name = GetUntilSplitter(request_str, ' ');
    const auto it = STR_TO_INPUT_REQUEST.find(request_name);
    if(it!=STR_TO_INPUT_REQUEST.end()){
        return it->second;
    }
    return nullopt;
}

RequestHolder ParseRequest(string_view request_str){
    optional<InputRequest::Type> request_type = GetInputRequestType(request_str);
    if(!request_type){
        throw invalid_argument("UNKNOWN REQUEST NAME");
    }
    RequestHolder request = InputRequest::Create(request_type.value());
    if(!request){throw invalid_argument("UNKNOWN REQUEST TYPE");}
    request -> ParseLine(request_str);
    return request;
}

vector<RequestHolder> ReadInputRequests(istream& in_stream){
  size_t request_count;
  in_stream >> request_count;
  vector<RequestHolder> requests;
  requests.reserve(request_count);
  for(size_t i=0; i<request_count; i++){
      string request_str;
      getline(in_stream, request_str);
      requests.push_back(move(ParseRequest(request_str)));
  }
  return requests;
}
*/

int main(){
    TestRunner tr;
    RUN_TEST(tr, TestParcingRequest);
    RUN_TEST(tr, TestReadInput);
    return 0;
}
