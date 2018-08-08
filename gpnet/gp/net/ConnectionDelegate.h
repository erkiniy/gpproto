//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#ifndef GPPROTO_CONNECTIONDELEGATE_H
#define GPPROTO_CONNECTIONDELEGATE_H

#include "Connection.h"
#include "gp/utils/StreamSlice.h"

namespace gpproto {
    class Connection;

    class ConnectionDelegate {
    public:
        virtual void connectionOpened(Connection& connection) = 0;
        virtual void connectionClosed(Connection& connection) = 0;
        virtual void connectionDidReceiveData(std::shared_ptr<StreamSlice>) = 0;
    };
}

#endif //GPPROTO_CONNECTIONDELEGATE_H
