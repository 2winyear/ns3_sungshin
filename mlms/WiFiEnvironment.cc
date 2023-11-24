#include "WiFiEnvironment.h"

WifiEnvironment::WifiEnvironment() {
	YansWifiChannelHelper channelHelper;
	channelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	channelHelper.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
	phyHelper.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
	phyHelper.SetChannel (channelHelper.Create ());
}

void WifiEnvironment::setSsid(std::string ssidString, Ipv4Address base, Ipv4Mask netmask) {
	ssid = Ssid(ssidString);
 	address.SetBase(base, netmask);
}

void WifiEnvironment::setWifiManager(std::string raa) {
	wifiHelper.SetRemoteStationManager("ns3::" + raa + "WifiManager");
}

void WifiEnvironment::setWifiChannelSetting(WifiStandard standard, std::string phyBandString, uint8_t channelNumber) {
	wifiHelper.SetStandard(standard);

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

	NS_ABORT_MSG_IF (channelWidth != 20 && channelWidth != 40 && channelWidth != 80 && channelWidth != 160, "Invalid channel width for standard " << standard);

	TupleValue<UintegerValue, UintegerValue, EnumValue, UintegerValue> channelValue;
	channelValue.Set (WifiPhy::ChannelTuple {channelNumber, channelWidth, band, 0});
	phyHelper.Set ("ChannelSettings", channelValue);
	phyHelper.SetErrorRateModel ("ns3::TableBasedErrorRateModel");
}

void WifiEnvironment::setMacType(std::string type) {
		macHelper.SetType (type, 
        	"QosSupported", BooleanValue (true),
        	"Ssid", SsidValue (ssid));
}

void WifiEnvironment::installStack(NodeContainer wifiNodes) {
	if(wifiNodes.Get(0)->GetObject<Ipv4>()) {
		std::cout << "it has\n";
	} else {
		stack.Install(wifiNodes);
	}
}

Ipv4InterfaceContainer WifiEnvironment::installWifi(NodeContainer wifiNodes) {
	NetDeviceContainer netDevices = wifiHelper.Install (phyHelper, macHelper, wifiNodes);

	if(wifiNodes.Get(0)->GetObject<Ipv4>()) {
		//std::cout << "it has\n";
	} else {
		stack.Install(wifiNodes);
	}

	Ipv4InterfaceContainer nodeInterfaces = address.Assign (netDevices);
	
	phyHelper.EnablePcap(ssid.PeekString(), netDevices);

	for(NodeContainer::Iterator itr = wifiNodes.Begin(); itr != wifiNodes.End(); ++itr) {
		Ptr<Node> node = *itr;

		Ptr<NetDevice> dev = DynamicCast<WifiNetDevice>(node->GetDevice(0));//->GetRemoteStationManager();
		Ptr<WifiRemoteStationManager> wifiManager = DynamicCast<WifiNetDevice>(dev)->GetRemoteStationManager();
		//wifiManager->AddBasicMode (WifiMode ("DsssRate11Mbps"));
		//wifiManager->GetMac();
	}
	//Ipv4InterfaceContainer nodeInterfaces;
	return nodeInterfaces;
}