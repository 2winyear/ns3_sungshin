#ifndef UDP_SINK_H
#define UDP_SINK_H

#include "ns3/tuple.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"

namespace ns3 {
    class UDPSink {
        public:
            Ptr<Socket> Install (Ptr<Node> node);
        private:
            uint32_t m_bytesTotal;    //!< Total number of received bytes.

    }


}
#endif