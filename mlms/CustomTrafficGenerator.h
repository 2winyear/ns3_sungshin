#ifndef CUSTOM_TRAFFIC_GENERATOR_H
#define CUSTOM_TRAFFIC_GENERATOR_H
 
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

#include "ns3/node-container.h"
#include "ns3/application-container.h"

#include "ns3/random-variable-stream.h"

namespace ns3 {
 
class Socket;
class Packet;
 
class CustomTrafficGenerator : public Application
{
public:
  static TypeId GetTypeId (void);
 
  CustomTrafficGenerator ();
 
  virtual ~CustomTrafficGenerator ();
 
  void SetRemote (Address ip, uint16_t port);
  void SetRemote (Address addr);
 
  uint64_t GetTotalTx () const;
 
protected:
  virtual void DoDispose (void);
 
private:
 
  virtual void StartApplication (void);
  virtual void StopApplication (void);
 
  void Send (void);
 
  uint32_t m_count; 

  Time m_interval; 
  bool b_random_interval;
  Ptr<ExponentialRandomVariable> m_random_interval;

  uint32_t m_size; 
  bool b_random_size;
  Ptr<ExponentialRandomVariable> m_random_size;

  uint32_t m_tos;

  uint32_t m_sent; 
  uint64_t m_totalTx; 
  Ptr<Socket> m_socket; 
  Address m_peerAddress; 
  uint16_t m_peerPort; 
  EventId m_sendEvent; 
#ifdef NS3_LOG_ENABLE
  std::string m_peerAddressString; 
#endif // NS3_LOG_ENABLE
};

class CustomTrafficGeneratorHelper
{
 
public:
  CustomTrafficGeneratorHelper ();
 
  CustomTrafficGeneratorHelper (Address ip, uint16_t port);
  CustomTrafficGeneratorHelper (Address addr);
 
  void SetAttribute (std::string name, const AttributeValue &value);
 
  ApplicationContainer Install (NodeContainer c);
 
private:
  ObjectFactory m_factory; 
};


} // namespace ns3
 
#endif /* CUSTOM_TRAFFIC_GENERATOR_Hs */