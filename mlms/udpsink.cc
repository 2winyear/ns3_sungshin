#include "udpsink.h"

namespace ns3 {
    UDPSink::UDPSink(){
    }

    Ptr<Socket> UDPSink::Install (Ptr<Node> node)
    {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ptr<Socket> sink = Socket::CreateSocket (node, tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
        sink->Bind (local);
        sink->SetRecvCallback (MakeCallback (&Experiment::ReceivePacket, this));
        
        m_bytesTotal = 0;

        return sink;
    }

    void UDPSink::ReceivePacket (Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while((packet = socket->Recv ())) {
            m_bytesTotal += packet->GetSize ();
        }
    }

}