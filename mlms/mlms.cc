#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/double.h"

#include "ns3/mobility-helper.h"

#include "ns3/udp-client-server-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-helper.h"

#include "ns3/wifi-remote-station-manager.h"

#include "CustomTrafficGenerator.h"

#include "Configuration.h"
#include "WiFiEnvironment.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("wifi-mlms");

void print(std::list<std::string> const &list)
{
    for (auto const &i: list) {
        std::cout << i << std::endl;
    }
}

int main (int argc, char *argv[])
{
	std::string configurationFileName = "wifi-mlms-test.json";

	CommandLine cmd (__FILE__);
	cmd.AddValue ("config", "Configuraition JSON filename", configurationFileName);
	cmd.Parse (argc,argv);

	WifiEnvironment wifiEnvironment;

	MobilityHelper mobility;

	Configuration configuration;
	configuration.readFromFile(configurationFileName);

	double simulationTime = configuration.getSimulationTime(); //seconds
    uint32_t payloadSize = 1472;//networks["payloadSize"].asInt(); //bytes

	// 네트워크 설정 적용. 노드 생성
	std::map<std::string, Network_t>::iterator network_itr = configuration.networkMap.begin();
	while(network_itr != configuration.networkMap.end()) {
		std::string ssid = network_itr->second.ssid;

		wifiEnvironment.setWifiManager(network_itr->second.wifiManager);

		NodeContainer_t AP_ContainerInfo = configuration.AP_ContainerMap[ssid];

		mobility.SetPositionAllocator(AP_ContainerInfo.positionAlloc);
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install(AP_ContainerInfo.nodeContainer);

		std::map<std::string, NodeContainer_t> bandToStaContainerMap = configuration.Sta_ContainerMap[ssid];

		std::map<std::string, Link_t> bandToLinkMap = network_itr->second.bandToLinkMap;
		std::map<std::string, Link_t>::iterator link_itr = bandToLinkMap.begin();
		while(link_itr != bandToLinkMap.end()) {
			std::string band = link_itr->first;
			std::string link_ssid = ssid + "-" + band;
			wifiEnvironment.setSsid(link_ssid, 
								link_itr->second.Ipv4Address.c_str(), 
								link_itr->second.Ipv4Mask.c_str());
			wifiEnvironment.setWifiChannelSetting(configuration.getStandardFromString(network_itr->second.standard), band, link_itr->second.channel);
			wifiEnvironment.setMacType("ns3::ApWifiMac");

			Ipv4InterfaceContainer interfaceContainer = wifiEnvironment.installWifi(AP_ContainerInfo.nodeContainer);
			link_itr->second.AP_IP = interfaceContainer.GetAddress(0).Get();
			
			//configuration.AP_ContainerMap[ssid] = AP_ContainerInfo; 	

			if( bandToStaContainerMap.find(band) != bandToStaContainerMap.end() ) {
				NodeContainer_t Sta_ContainerInfo = bandToStaContainerMap[band];

				std::cout << "* ssid: " << ssid << " (" << link_ssid << ", " << Ipv4Address(link_itr->second.AP_IP) << "), Sta: " << Sta_ContainerInfo.nNodes << "\n";
				Sta_ContainerInfo.nodeContainer.Create(Sta_ContainerInfo.nNodes);

				wifiEnvironment.setMacType("ns3::StaWifiMac");

				mobility.SetPositionAllocator(Sta_ContainerInfo.positionAlloc);
				mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
				mobility.Install(Sta_ContainerInfo.nodeContainer);

				wifiEnvironment.installWifi(Sta_ContainerInfo.nodeContainer);
				bandToStaContainerMap[band] = Sta_ContainerInfo;
				configuration.Sta_ContainerMap[ssid] = bandToStaContainerMap;

				for(uint32_t i=0; i < Sta_ContainerInfo.nodeContainer.GetN(); i++) {
					Ptr<Node> destNode = Sta_ContainerInfo.nodeContainer.Get(i);
					Ptr<Ipv4> ipv4 = destNode->GetObject<Ipv4>();
					std::cout << "\tsta " << i << ": " << ipv4->GetAddress(1, 0).GetAddress() << "\n";
				}
			}

			//configuration.AP_ContainerMap[network_itr->second.ssid] = nodeContainerInfo; 	
			link_itr++;
		}
		network_itr->second.bandToLinkMap = bandToLinkMap;

		network_itr++;
	}

	// 애플리케이션 설치
	std::map<std::string, ApplicationContainer> serverAppContainerMap;

	std::list<Application_t> applicationList = configuration.getApplicationList();
	std::list<Application_t>::iterator list_itr = applicationList.begin();
	while( list_itr != applicationList.end() ) {
		std::cout << "* Application from " << list_itr->ssid << "-" << list_itr->band  << ", ("  << list_itr->srcType << ", " << list_itr->srcIndex << ") to (" << list_itr->destType << ", " << list_itr->destIndex << ", port: " << list_itr->destPort << ") " << std::endl;
	
		Ptr<Node> srcNode;
		if(list_itr->srcType == "Ap") {
			NodeContainer_t srcInfo = configuration.AP_ContainerMap.find(list_itr->ssid)->second;
			srcNode = srcInfo.nodeContainer.Get(0);
		} else {
			NodeContainer_t srcInfo = configuration.Sta_ContainerMap.find(list_itr->ssid)->second.find(list_itr->band)->second;
			srcNode = srcInfo.nodeContainer.Get(list_itr->srcIndex);
		} 

		// AP 설치 (서버)
		if(list_itr->destType == "Ap" || (list_itr->srcType != "Ap" && list_itr->destType == "Broadcast")) {
			std::stringstream serverAppKeyStream;
			serverAppKeyStream << list_itr->ssid << "-" << list_itr->band << "/Ap/" << list_itr->destPort;
			std::string serverAppKey = serverAppKeyStream.str();
			if( serverAppContainerMap.find(serverAppKey) == serverAppContainerMap.end() ) {
				UdpServerHelper udpServer(list_itr->destPort);
				ApplicationContainer serverApp = udpServer.Install(configuration.AP_ContainerMap.find(list_itr->ssid)->second.nodeContainer);
				serverApp.Start(Seconds (0.0));
				serverApp.Stop(Seconds (simulationTime + 1));
				serverAppContainerMap.insert(std::make_pair(serverAppKey, serverApp));
				std::cout << "\t" << serverAppKey << " server\n";
			}
		}

		// STA 설치 (서버)
		if(list_itr->destType == "Sta" || list_itr->destType == "Broadcast" ) {
			if(configuration.Sta_ContainerMap.find(list_itr->ssid) != configuration.Sta_ContainerMap.end()) {
				std::map<std::string, NodeContainer_t> bandToStaContainerMap = configuration.Sta_ContainerMap[list_itr->ssid];

				if(bandToStaContainerMap.find(list_itr->band) != bandToStaContainerMap.end() ) {
					NodeContainer_t staContainerInfo = bandToStaContainerMap[list_itr->band];
					for(uint32_t temp = 0; temp < staContainerInfo.nodeContainer.GetN() ; temp++) {
						if(list_itr->destType == "Sta" && temp != list_itr->destIndex ) {
							continue;
						}
						std::stringstream serverAppKeyStream;
						serverAppKeyStream << list_itr->ssid << "-" << list_itr->band  << "/Sta/" << temp << "/" << list_itr->destPort;
						std::string serverAppKey = serverAppKeyStream.str();
						if ( serverAppContainerMap.find(serverAppKey) == serverAppContainerMap.end() ) {
							UdpServerHelper udpServer(list_itr->destPort);
							ApplicationContainer serverApp = udpServer.Install(staContainerInfo.nodeContainer.Get(temp));
							serverApp.Start(Seconds (0.0));
							serverApp.Stop(Seconds (simulationTime + 1));
							serverAppContainerMap.insert(std::make_pair(serverAppKey, serverApp));
							std::cout << "\t" << serverAppKey << " server\n";
						} 
					}
				}
			}
		}

		Ipv4Address destIP;
		if( list_itr->destType == "Broadcast" ) {
			destIP = Ipv4Address("255.255.255.255");
		} else if( list_itr->destType == "Ap" ) {
			destIP = Ipv4Address( configuration.networkMap.find(list_itr->ssid)->second.bandToLinkMap.find(list_itr->band)->second.AP_IP );
		} else {
			NodeContainer_t destInfo = configuration.Sta_ContainerMap.find(list_itr->ssid)->second.find(list_itr->band)->second;

			Ptr<Node> destNode = destInfo.nodeContainer.Get(list_itr->destIndex);
			Ptr<Ipv4> ipv4 = destNode->GetObject<Ipv4>();
			//std::cout << ipv4->GetAddress(1, 0).GetAddress() << "\n";
			//std::cout << destNode->GetDevice(0)->GetAddress() << std::endl;
			//socketAddr.SetPhysicalAddress (destNode->GetDevice(0)->GetAddress ());
			destIP = ipv4->GetAddress(1, 0).GetAddress();
		}

		std::cout << "\t- Destnation: " << destIP << ":" << list_itr->destPort << "\n";
		if(list_itr->AC != "") {
			std::cout << "\tAC : AC_" << list_itr->AC << "\n";
		}
		uint32_t tos = 0;
		if(list_itr->AC == "BE") {
			tos = 0x70;
		} else if(list_itr->AC == "BK"){
			tos = 0x28;
		} else if(list_itr->AC == "VI"){
			tos = 0xb8;
		} else if(list_itr->AC == "VO"){
			tos = 0xc0;
		}

		// Traffic Generator 설치 (클라이언트)
		CustomTrafficGeneratorHelper udpClient(destIP, list_itr->destPort);
		udpClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
		udpClient.SetAttribute ("Interval", TimeValue (Time (list_itr->interval * 1000000000))); // interval (s) between packets 
		udpClient.SetAttribute ("RandomInterval", BooleanValue (list_itr->randomInterval)); // randomness?
		udpClient.SetAttribute ("PacketSize", UintegerValue (list_itr->packetSize)); //bytes
		udpClient.SetAttribute ("RandomPacketSize", BooleanValue (list_itr->randomPacketSize));
		udpClient.SetAttribute ("Tos", UintegerValue (tos)); 
		ApplicationContainer clientApp = udpClient.Install(srcNode);


		clientApp.Start(Seconds (1.0));
		clientApp.Stop(Seconds (simulationTime + 1));
		list_itr++;
	}

	// not sure whether required
	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	Simulator::Stop (Seconds (simulationTime + 5));

	FlowMonitorHelper flowmon_helper = FlowMonitorHelper();
	Ptr<FlowMonitor> monitor = flowmon_helper.InstallAll();
	monitor->SetAttribute("DelayBinWidth", DoubleValue (0.001));
	monitor->SetAttribute("JitterBinWidth", DoubleValue (0.001));
	monitor->SetAttribute("PacketSizeBinWidth", DoubleValue (20));

	Simulator::Run ();

	monitor->SerializeToXmlFile("results.xml", true, true);

	std::map<std::string, ApplicationContainer>::iterator itr = serverAppContainerMap.begin();

	while(itr != serverAppContainerMap.end()) {
		uint64_t rxBytes;//, nLost;
		double throughput;

		rxBytes = payloadSize * DynamicCast<UdpServer> (itr->second.Get(0))->GetReceived ();
		if( rxBytes > 0 ) {
			//nLost = DynamicCast<UdpServer> (itr->second.Get(0))->GetLost();
			throughput = (rxBytes * 8) / (simulationTime * 1000000.0); //Mbit/s
			std::cout << "----------------------" << std::endl << itr->first << std::endl << "----------------------" << std::endl;
			std::cout << " Received Throughput: " << throughput << " Mbit/s" << std::endl;
			//std::cout << " lost: " << nLost << std::endl;
		} else {
			//std::cout << itr->first << " ???\n";
		}
		itr++;
	}

	Simulator::Destroy ();

	return 0;
}
