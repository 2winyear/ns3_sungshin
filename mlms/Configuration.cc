#include "Configuration.h"

Configuration::Configuration() {
    simulationTime = 0;
    networkMap = std::map<std::string, Network_t>();
    nodeContainerMap = std::map<std::string, std::map<std::string, NodeContainer_t>>();
}

double Configuration::getSimulationTime() {
    return simulationTime;
}

std::list<Application_t> Configuration::getApplicationList() {
    return applicationList;
}

WifiStandard Configuration::getStandardFromString(std::string standard) {
    if(standard == "802.11ac" ) {
        return WIFI_STANDARD_80211ac;
    } else if(standard == "802.11ax" ) {
        return WIFI_STANDARD_80211ax;
    } else {
        return WIFI_STANDARD_80211ac;
    }
}

int Configuration::readFromFile(std::string filename) {
    std::ifstream ist(filename);
    std::string str;

    for (char p; ist >> p;) {
        str += p;
    }

    Json::Value root;
    Json::Reader reader;
	bool parsingRet = reader.parse(str, root);
	if (!parsingRet) {
		std::cout << "Incorrect JSON" << std::endl;
		return 1;
	}

	for(std::string id : root.getMemberNames()) {
        if(id == "simulationTime") {
            simulationTime = root[id].asDouble();
        } else if(id == "networks") {
            Json::Value networks = root[id];
            Json::ValueIterator itr = networks.begin();
            while(itr != networks.end()) {
                if(itr->isObject()) {
                    Network_t networkInfo;
                    std::string ssid = (*itr)["ssid"].asCString();
                    networkInfo.ssid = ssid;
                    networkInfo.standard = (*itr)["standard"].asCString();
                    networkInfo.wifiManager = (*itr)["wifiManager"].asCString();
                    
                    Json::Value AP = (*itr)["AP"];
                    Vector AP_locationVector = Vector(AP["location"][0].asFloat(), AP["location"][1].asFloat(), AP["location"][2].asFloat());
                    Json::Value links = AP["links"];
                    Json::ValueIterator link_itr = links.begin();
                    while(link_itr != links.end()) {
                        Link_t linkInfo;
                        std::string channelBand = (*link_itr)["band"].asCString();
                        linkInfo.channel = (*link_itr)["channel"].asInt();
                        linkInfo.Ipv4Address = (*link_itr)["Ipv4Address"].asCString();
                        linkInfo.Ipv4Mask = (*link_itr)["Ipv4Mask"].asCString();
                        networkInfo.bandToLinkMap.insert(std::make_pair(channelBand, linkInfo));
                        link_itr++;
                    }
                    std::map<std::string, NodeContainer_t>::iterator map_itr = AP_ContainerMap.find(ssid);
                    if(map_itr != AP_ContainerMap.end()) {             
                        //map_itr->second.nNodes = map_itr->second.nNodes + 1;
                        //map_itr->second.positionAlloc->Add(AP_locationVector);
                        std::cout << "cannot be here!!!!!\n";
                    } else {
                        NodeContainer_t nodeContainerInfo;
                        nodeContainerInfo.nNodes = 1;
                        nodeContainerInfo.nodeContainer.Create(1);
                        nodeContainerInfo.positionAlloc = CreateObject<ListPositionAllocator> ();
                        nodeContainerInfo.positionAlloc->Add(AP_locationVector);
                  
                        AP_ContainerMap.insert(std::make_pair(ssid, nodeContainerInfo));
                    }

                    if( itr->isMember("AP_applications") ) {
                        Json::Value applications = (*itr)["AP_applications"];
                        Json::ValueIterator applications_itr = applications.begin();

                        while(applications_itr != applications.end()) {
                            Json::Value application = (*applications_itr);
                            Application_t applicationInfo;
                            applicationInfo.ssid = ssid;
                            applicationInfo.band = application["band"].asCString();

                            if(application.isMember("randomInterval")) {
                                applicationInfo.randomInterval = application["randomInterval"].asBool();
                            } else {
                                applicationInfo.randomInterval = false;
                            }
                            applicationInfo.interval = application["interval"].asFloat();
                            applicationInfo.srcType = "Ap";
                            applicationInfo.srcIndex = 0;
                            applicationInfo.destType = application["destType"].asCString();
                            if(applicationInfo.destType != "Broadcast") {
                                applicationInfo.destIndex = application["destIndex"].asInt();
                            }   else {
                                applicationInfo.destIndex = 0;
                            }
                            applicationInfo.destPort = application["destPort"].asInt();
                            if(application.isMember("randomPacketSize")) {
                                applicationInfo.randomPacketSize = application["randomPacketSize"].asBool();
                            } else {
                                applicationInfo.randomPacketSize = false;
                            }
                            applicationInfo.packetSize = application["packetSize"].asInt();

                            if(application.isMember("AC")) {
                                applicationInfo.AC = application["AC"].asCString();
                            } else {
                                applicationInfo.AC = "";
                            }

                            applicationList.push_back(applicationInfo);
                            applications_itr++;
                        }
                    }

                    std::map<std::string, Network_t>::iterator networkMap_itr = networkMap.find(ssid);
                    if(networkMap_itr != networkMap.end()) {
                        std::cerr << "How do we already have it???" << std::endl;
                    } else {
                        networkMap.insert(std::make_pair(ssid, networkInfo));
                    }

                } else {
                    std::cerr << "Network is not Object????" << std::endl;
                }
                itr++;
            }
        } else if(id == "stations") {
            Json::Value nodes = root[id];
            Json::ValueIterator itr = nodes.begin();
            while(itr != nodes.end()) {                
                if(itr->isObject()) {
                    std::string ssid = (*itr)["ssid"].asCString();
                    std::string band = (*itr)["band"].asCString();

                    Vector locationVector = Vector((*itr)["location"][0].asFloat(), (*itr)["location"][1].asFloat(), (*itr)["location"][2].asFloat());
                   
                    if(Sta_ContainerMap.find(ssid) != Sta_ContainerMap.end()) {
		                std::map<std::string, NodeContainer_t> bandToStaContainerMap = Sta_ContainerMap[ssid];

                        //std::map<std::string, NodeContainer_t>::iterator inner_itr = sta_map_itr->second.find(band);
                        if(bandToStaContainerMap.find(band) != bandToStaContainerMap.end()) {
                            NodeContainer_t staInfo = bandToStaContainerMap[band];
                            staInfo.nNodes = staInfo.nNodes + 1;
                            staInfo.positionAlloc->Add(locationVector);
                            bandToStaContainerMap[band] = staInfo;
                            Sta_ContainerMap[ssid] = bandToStaContainerMap;
                        } else {
                            NodeContainer_t nodeContainerInfo;
                            nodeContainerInfo.nNodes = 1;
                            nodeContainerInfo.positionAlloc = CreateObject<ListPositionAllocator> ();
                            nodeContainerInfo.positionAlloc->Add(locationVector);
                            bandToStaContainerMap[band] = nodeContainerInfo;
                            //sta_map_itr->second.insert(std::make_pair(band, nodeContainerInfo));
                            Sta_ContainerMap[ssid] = bandToStaContainerMap;
                        }
                    } else {
                        NodeContainer_t nodeContainerInfo;
                        nodeContainerInfo.nNodes = 1;
                        nodeContainerInfo.positionAlloc = CreateObject<ListPositionAllocator> ();
                        nodeContainerInfo.positionAlloc->Add(locationVector);

                        std::map<std::string, NodeContainer_t> bandToStaContainerMap;
                        bandToStaContainerMap.insert(std::make_pair(band, nodeContainerInfo));
                        Sta_ContainerMap.insert(std::make_pair(ssid, bandToStaContainerMap));
                    }

                    if( itr->isMember("application") ) {
                        Json::Value application = (*itr)["application"];
                        Application_t applicationInfo;
                        applicationInfo.ssid = ssid;
                        applicationInfo.band = band;
                        if(application.isMember("randomInterval")) {
                            applicationInfo.randomInterval = application["randomInterval"].asBool();
                        } else {
                            applicationInfo.randomInterval = false;
                        }
                        applicationInfo.interval = application["interval"].asFloat();
                        applicationInfo.srcType = "Sta";
                        applicationInfo.srcIndex = Sta_ContainerMap.find(ssid)->second.find(band)->second.nNodes - 1;
                        applicationInfo.destType = application["destType"].asCString();
                        if(applicationInfo.destType != "Broadcast") {
                            applicationInfo.destIndex = application["destIndex"].asInt();
                        } else {
                            applicationInfo.destIndex = 0;
                        }
                        applicationInfo.destPort = application["destPort"].asInt();
                        if(application.isMember("randomPacketSize")) {
                            applicationInfo.randomPacketSize = application["randomPacketSize"].asBool();
                        } else {
                            applicationInfo.randomPacketSize = false;
                        }
                        applicationInfo.packetSize = application["packetSize"].asInt();
                        if(application.isMember("AC")) {
                                applicationInfo.AC = application["AC"].asCString();
                            } else {
                                applicationInfo.AC = "";
                            }
                        applicationList.push_back(applicationInfo);
                    }
                } else {
                    std::cerr << "What???" << std::endl;
                }
                
                itr++;
            }
        }
	}
    return 0;
}