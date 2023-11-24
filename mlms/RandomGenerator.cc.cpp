#include "RandomGenerator.h"

TypeId
RandomGenerator::GetTypeId (void)
{
  static TypeId tid = TypeId ("RandomGenerator")
    .SetParent<Application> ()
    .AddConstructor<RandomGenerator> ()
    .AddAttribute ("Delay", "The delay between two packets (s)",
           RandomVariableValue (ConstantVariable (1.0)),
           MakeRandomVariableAccessor (&RandomGenerator::m_delay),
           MakeRandomVariableChecker ())
    .AddAttribute ("PacketSize", "The size of each packet (bytes)",
           RandomVariableValue (ConstantVariable (2000)),
           MakeRandomVariableAccessor (&RandomGenerator::m_size),
           MakeRandomVariableChecker ())
    ;
  return tid;
}   

void 
RandomGenerator::SetRemote (std::string socketType, 
                            Address remote)
{
  TypeId tid = TypeId::LookupByName (socketType);
  m_socket = Socket::CreateSocket (GetNode (), tid);
  m_socket->Bind ();
  m_socket->ShutdownRecv ();
  m_socket->Connect (remote);
}

void
RandomGenerator::DoGenerate (void)
{
  m_next = Simulator::Schedule (Seconds (m_delay.GetValue ()), 
                &RandomGenerator::DoGenerate, this);
  Ptr<Packet> p = Create<Packet> (m_size.GetIntValue ());
  m_socket->Send (p);
}

void 
RandomAppHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}
ApplicationContainer 
RandomAppHelper::Install (NodeContainer nodes)
{
  ApplicationContainer applications;
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Ptr<RandomAppHelper> app = m_factory.Create<RandomAppHelper> ();
      app->SetRemote (m_socketType, m_remote);
      (*i)->AddApplication (app);
      applications.Add (app);
    }
  return applications;
}