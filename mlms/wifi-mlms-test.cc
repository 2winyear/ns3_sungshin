#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/tuple.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/ht-configuration.h"
#include "ns3/he-configuration.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("wifi-mlms-test");

class WifiEnvironment {
private:
	YansWifiPhyHelper phy; 
	
 	WifiMacHelper mac;
 	WifiHelper wifi;
	Ssid ssid; 

	InternetStackHelper stack;
	Ipv4AddressHelper address;
public:
	WifiEnvironment();
	void setSsid(std::string ssidString, Ipv4Address base, Ipv4Mask netmask);
	void setWifiChannelSetting(WifiStandard standard, std::string phyBandString);
	void setMacType(std::string type);
	void setWifiManager(std::string raa);
	Ipv4InterfaceContainer installWifiAndIP(NodeContainer wifiNode);
};

WifiEnvironment::WifiEnvironment() {
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
	phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
	phy.SetChannel (channel.Create ());
}

void WifiEnvironment::setSsid(std::string ssidString, Ipv4Address base, Ipv4Mask netmask) {
	ssid = Ssid(ssidString);
 	address.SetBase (base, netmask);
}

void WifiEnvironment::setWifiManager(std::string raa) {
	wifi.SetRemoteStationManager ("ns3::" + raa + "WifiManager");
}

void WifiEnvironment::setWifiChannelSetting(WifiStandard standard, std::string phyBandString) {
	wifi.SetStandard(standard);

	WifiPhyBand band = WIFI_PHY_BAND_2_4GHZ;

	if(phyBandString == "2.4Ghz") {
		if(standard == WIFI_STANDARD_80211ac) {
			std::cerr << phyBandString << " not allowed\n";
			band = WIFI_PHY_BAND_5GHZ;
		}  
	} else if(phyBandString == "5Ghz") {
		if(standard == WIFI_STANDARD_80211n || standard == WIFI_STANDARD_80211ac || standard == WIFI_STANDARD_80211ax) {
			band = WIFI_PHY_BAND_5GHZ;
		} else {
			std::cerr << phyBandString << " not allowed\n";      
		}
	} else if(phyBandString == "6Ghz") {
		if(standard == WIFI_STANDARD_80211ax) {
			band = WIFI_PHY_BAND_6GHZ;
		} else {
			std::cerr << phyBandString << " not allowed\n";
		}
	}

	uint16_t channelWidth = GetDefaultChannelWidth (standard, band);
	TupleValue<UintegerValue, UintegerValue, EnumValue, UintegerValue> channelValue;
	channelValue.Set (WifiPhy::ChannelTuple {0, channelWidth, band, 0});
	phy.Set ("ChannelSettings", channelValue);
	phy.SetErrorRateModel ("ns3::TableBasedErrorRateModel");
}

void WifiEnvironment::setMacType(std::string type) {
		mac.SetType (type, 
        "QosSupported", BooleanValue (true),
        "Ssid", SsidValue (ssid));
}

Ipv4InterfaceContainer WifiEnvironment::installWifiAndIP(NodeContainer wifiNodes) {
	NetDeviceContainer netDevices = wifi.Install (phy, mac, wifiNodes);
	stack.Install(wifiNodes);
	Ipv4InterfaceContainer nodeInterfaces = address.Assign (netDevices);
	return nodeInterfaces;
}

int main (int argc, char *argv[])
{
	uint32_t payloadSize = 1472; //bytes
	double simulationTime = 10; //seconds

	bool apHasTraffic = false;
	bool staHasTraffic = true;

	CommandLine cmd (__FILE__);
	cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
	cmd.AddValue ("apHasTraffic", "Enable/disable traffic on the AP", apHasTraffic);
	cmd.AddValue ("staHasTraffic", "Enable/disable traffic on the station", staHasTraffic);
	cmd.Parse (argc,argv);

	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (2);
	NodeContainer wifiApNodes;
	wifiApNodes.Create (1);

	NodeContainer wifiStaNodes2;
	wifiStaNodes2.Create (1);
	NodeContainer wifiApNodes2;
	wifiApNodes2.Create (1);

	WifiEnvironment wifiEnvironment;

	wifiEnvironment.setSsid("ns3", "192.168.1.0", "255.255.255.0");
	wifiEnvironment.setWifiManager("Ideal");
	wifiEnvironment.setWifiChannelSetting(WIFI_STANDARD_80211ac, "5Ghz");

	wifiEnvironment.setMacType("ns3::StaWifiMac");
	Ipv4InterfaceContainer staNodeInterface = wifiEnvironment.installWifiAndIP(wifiStaNodes);

	wifiEnvironment.setMacType("ns3::ApWifiMac");
	Ipv4InterfaceContainer apNodeInterface = wifiEnvironment.installWifiAndIP(wifiApNodes);

	wifiEnvironment.setSsid("ns3-2", "192.168.2.0", "255.255.255.0");
	wifiEnvironment.setWifiChannelSetting(WIFI_STANDARD_80211ac, "5Ghz");
	wifiEnvironment.setMacType("ns3::StaWifiMac");
	Ipv4InterfaceContainer staNodeInterface2 = wifiEnvironment.installWifiAndIP(wifiStaNodes2);

	wifiEnvironment.setMacType("ns3::ApWifiMac");
	Ipv4InterfaceContainer apNodeInterface2 = wifiEnvironment.installWifiAndIP(wifiApNodes2);

	// mobility - location
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (5.0, 0.0, 0.0));
	positionAlloc->Add (Vector (5.0, 0.0, 0.0));
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (5.0, 0.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobility.Install (wifiApNodes);
	mobility.Install (wifiStaNodes);
	mobility.Install (wifiApNodes2);    
	mobility.Install (wifiStaNodes2);


	// application setup
	UdpServerHelper apServer (9);
	ApplicationContainer apServerApp = apServer.Install (wifiApNodes.Get (0));
	apServerApp.Start (Seconds (0.0));
	apServerApp.Stop (Seconds (simulationTime + 1));

	UdpServerHelper apServer2 (9);
	ApplicationContainer apServerApp2 = apServer2.Install (wifiApNodes2.Get (0));
	apServerApp2.Start (Seconds (0.0));
	apServerApp2.Stop (Seconds (simulationTime + 1));

	UdpServerHelper staServer (5001);
	ApplicationContainer staServerApp = staServer.Install (wifiStaNodes.Get (0));
	staServerApp.Start (Seconds (0.0));
	staServerApp.Stop (Seconds (simulationTime + 1));

	if (apHasTraffic) {
		UdpClientHelper apClient (staNodeInterface.GetAddress (0), 5001);
		apClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
		apClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
		apClient.SetAttribute ("PacketSize", UintegerValue (payloadSize)); //bytes
		ApplicationContainer apClientApp = apClient.Install (wifiApNodes.Get (0));
		apClientApp.Start (Seconds (1.0));
		apClientApp.Stop (Seconds (simulationTime + 1));
	}

	if (staHasTraffic) {
		UdpClientHelper staClient (apNodeInterface.GetAddress (0), 9);
		staClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
		staClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
		staClient.SetAttribute ("PacketSize", UintegerValue (payloadSize)); //bytes
		ApplicationContainer staClientApp = staClient.Install (wifiStaNodes.Get (0));
		staClientApp.Start (Seconds (1.0));
		staClientApp.Stop (Seconds (simulationTime + 1));

		UdpClientHelper staClient2 (apNodeInterface2.GetAddress (0), 9);
		staClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
		staClient2.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
		staClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize)); //bytes
		ApplicationContainer staClientApp2 = staClient2.Install (wifiStaNodes2.Get (0));
		staClientApp2.Start (Seconds (1.0));
		staClientApp2.Stop (Seconds (simulationTime + 1));
	}

// not sure whether required
//  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	Simulator::Stop (Seconds (simulationTime + 1));
	Simulator::Run ();

	uint64_t rxBytes;
	double throughput;
	bool error = false;
	if (apHasTraffic) {
		rxBytes = payloadSize * DynamicCast<UdpServer> (staServerApp.Get (0))->GetReceived ();
		throughput = (rxBytes * 8) / (simulationTime * 1000000.0); //Mbit/s
		std::cout << "AP Throughput: " << throughput << " Mbit/s" << std::endl;
		if (throughput == 0) {
			error = true;
		}
	}
	if (staHasTraffic) {
		rxBytes = payloadSize * DynamicCast<UdpServer> (apServerApp.Get (0))->GetReceived ();
		throughput = (rxBytes * 8) / (simulationTime * 1000000.0); //Mbit/s
		std::cout << "STA Throughput: " << throughput << " Mbit/s" << std::endl;
		if (throughput == 0) {
			error = true;
		}
	}

	Simulator::Destroy ();

	if (error) {
		NS_LOG_ERROR ("No traffic received!");
		exit (1);
	}

	return 0;
}
