//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#ifndef GPPROTO_TCPTRANSPORTCONTEXTDELEGATE_H
#define GPPROTO_TCPTRANSPORTCONTEXTDELEGATE_H

#include "gp/network/TcpTransportContext.h"

namespace gpproto
{
    class TcpTransportContext;
    class TcpTransportContextDelegate {
    public:
        virtual void tcpConnectionRequestReconnection(const TcpTransportContext& context) = 0;
    };
}

#endif //GPPROTO_TCPTRANSPORTCONTEXTDELEGATE_H
