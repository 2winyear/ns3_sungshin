#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "ns3/ptr.h"
#include "ns3/position-allocator.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/wifi-standards.h"
#include "json/json.h"

#include <list>
#include <string>
#include <map>

using namespace ns3;

struct Link_t {
    int    channel;

    std::string Ipv4Address;
    std::string Ipv4Mask;
    uint32_t 	 AP_IP;
};

struct Network_t {
    std::string ssid;
    std::string standard;
    std::string wifiManager;

    std::map<std::string, Link_t> bandToLinkMap;
};

struct NodeContainer_t {
    int nNodes;
    NodeContainer nodeContainer;
    Ptr<ListPositionAllocator> positionAlloc;
};

struct EDCA_Parameter_t {
    int CWmin;
    int CWmax;
    int AIFNS;
    int MaxTXOP_ms;
};

struct Application_t {
    std::string ssid;
    std::string band;

    bool randomInterval;
    float interval;

    std::string srcType;
    uint32_t srcIndex;
    std::string destType;
    uint32_t destIndex;
    uint32_t destPort;

    bool randomPacketSize;
    uint32_t packetSize;

    std::string AC;
};

class Configuration {
private:
    double  simulationTime;
    std::list<Application_t> applicationList;


public:
    Configuration();
    
    double getSimulationTime();
    std::list<Application_t> getApplicationList();
    WifiStandard getStandardFromString(std::string standard);
    int readFromFile(std::string filename);

    std::map<std::string, Network_t> networkMap;
    std::map<std::string, std::map<std::string, NodeContainer_t>> nodeContainerMap;
    std::map<std::string, NodeContainer_t> AP_ContainerMap;
    std::map<std::string, std::map<std::string, NodeContainer_t>> Sta_ContainerMap;

};

#endif

