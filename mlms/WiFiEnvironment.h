#ifndef __WIFIENVIRONMENT_H__
#define __WIFIENVIRONMENT_H__

#include "ns3/tuple.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"

#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"

using namespace ns3;

class WifiEnvironment {
private:
	YansWifiPhyHelper phyHelper; 
	
 	WifiMacHelper macHelper;
 	WifiHelper wifiHelper;

	Ssid ssid; 
	InternetStackHelper stack;
	Ipv4AddressHelper address;

public:
	WifiEnvironment();
	void setSsid(std::string ssidString, Ipv4Address base, Ipv4Mask netmask);
	void setWifiChannelSetting(WifiStandard standard, std::string phyBandString, uint8_t channelNumber);
	void setMacType(std::string type);
	void setWifiManager(std::string raa);
	void installStack(NodeContainer wifiNodes);
	Ipv4InterfaceContainer installWifi(NodeContainer wifiNode);
};

#endif