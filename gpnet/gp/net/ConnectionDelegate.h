//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#ifndef GPPROTO_CONNECTIONDELEGATE_H
#define GPPROTO_CONNECTIONDELEGATE_H

#include <memory>

namespace gpproto {
    class Connection;
    class StreamSlice;

    class ConnectionDelegate {
    public:
        virtual void connectionOpened(const Connection& connection) = 0;
        virtual void connectionClosed(const Connection& connection) = 0;
        virtual void connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) = 0;
    };
}

#endif //GPPROTO_CONNECTIONDELEGATE_H
