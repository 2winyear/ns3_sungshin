#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include "ns3/boolean.h"
#include "ns3/double.h"

#include "CustomTrafficGenerator.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>
 
namespace ns3 {
 
NS_LOG_COMPONENT_DEFINE ("CustomTrafficGenerator");
 
NS_OBJECT_ENSURE_REGISTERED (CustomTrafficGenerator);
 
TypeId
CustomTrafficGenerator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CustomTrafficGenerator")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<CustomTrafficGenerator> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&CustomTrafficGenerator::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RandomInterval",
                   "Add randomness", 
                   BooleanValue (0),
                   MakeBooleanAccessor (&CustomTrafficGenerator::b_random_interval),
                   MakeBooleanChecker ())                   
    .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&CustomTrafficGenerator::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&CustomTrafficGenerator::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&CustomTrafficGenerator::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("RandomPacketSize",
                   "Add randomness", 
                   BooleanValue (0),
                   MakeBooleanAccessor (&CustomTrafficGenerator::b_random_size),
                   MakeBooleanChecker ())                    
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&CustomTrafficGenerator::m_size),
                   MakeUintegerChecker<uint32_t> (12,65507))
    .AddAttribute ("Tos",
                   "Tos for EDCA",
                   UintegerValue (0),
                   MakeUintegerAccessor (&CustomTrafficGenerator::m_tos),
                   MakeUintegerChecker<uint32_t> ())                   
  ;
  return tid;
}
 
CustomTrafficGenerator::CustomTrafficGenerator ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_totalTx = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
}
 
CustomTrafficGenerator::~CustomTrafficGenerator ()
{
  NS_LOG_FUNCTION (this);
}
 
void
CustomTrafficGenerator::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}
 
void
CustomTrafficGenerator::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}
 
void
CustomTrafficGenerator::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}
 
void
CustomTrafficGenerator::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
 
  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          InetSocketAddress dest = InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort);
          if(m_tos > 0) {
            dest.SetTos(m_tos);
            //std::cout << " tostest\n";
          }
          m_socket->Connect (dest);
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }

    }
 
#ifdef NS3_LOG_ENABLE
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 ();
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 ();
    }
  m_peerAddressString = peerAddressStringStream.str();
#endif // NS3_LOG_ENABLE
 
  m_random_interval = CreateObject<ExponentialRandomVariable> ();
  m_random_interval->SetAttribute ("Mean", DoubleValue (m_interval.GetNanoSeconds()));
  m_random_interval->SetAttribute ("Bound", DoubleValue (0));

  m_random_size = CreateObject<ExponentialRandomVariable> ();
  m_random_size->SetAttribute ("Mean", DoubleValue (m_size));
  m_random_size->SetAttribute ("Bound", DoubleValue (1472));  

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &CustomTrafficGenerator::Send, this);
}
 
void
CustomTrafficGenerator::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}
 
void
CustomTrafficGenerator::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);

  uint32_t size = 1;
  if(b_random_size) {
    size = (uint32_t) ( m_random_size->GetInteger() );
    size = std::max(size, (uint32_t) 22);
  } else {
    size = m_size;
  }

  Ptr<Packet> p = Create<Packet> (size-(8+4)); // 8+4 : the size of the seqTs header
  //Ptr<Packet> p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
  p->AddHeader (seqTs);
 
  if ((m_socket->Send (p)) >= 0)
    {
      ++m_sent;
      m_totalTx += p->GetSize ();
#ifdef NS3_LOG_ENABLE
    NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
                                    << m_peerAddressString << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).As (Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                                          << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE
 
  if (m_sent < m_count)
    {
      if(b_random_interval) {
        Time nextT = Time(m_random_interval->GetInteger());
        m_sendEvent = Simulator::Schedule (nextT, &CustomTrafficGenerator::Send, this);
        //std::cout << "random scheduling send " << nextT << " " << nextT.GetNanoSeconds()<< "\n";        
      } else {
        m_sendEvent = Simulator::Schedule (m_interval, &CustomTrafficGenerator::Send, this);
        //std::cout << "static scheduling send " << m_interval << " " << m_interval.GetNanoSeconds() << "\n";
      }
    }
}
  
uint64_t
CustomTrafficGenerator::GetTotalTx () const
{
  return m_totalTx;
}

CustomTrafficGeneratorHelper::CustomTrafficGeneratorHelper ()
{
  m_factory.SetTypeId (CustomTrafficGenerator::GetTypeId ());
}
 
CustomTrafficGeneratorHelper::CustomTrafficGeneratorHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (CustomTrafficGenerator::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}
 
CustomTrafficGeneratorHelper::CustomTrafficGeneratorHelper (Address address)
{
  m_factory.SetTypeId (CustomTrafficGenerator::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}
 
void
CustomTrafficGeneratorHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}
 
ApplicationContainer
CustomTrafficGeneratorHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<CustomTrafficGenerator> client = m_factory.Create<CustomTrafficGenerator> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}
 
} // Namespace ns3