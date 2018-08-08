//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#ifndef GPPROTO_CONNECTION_H
#define GPPROTO_CONNECTION_H

#include "ConnectionDelegate.h"
#include <list>

namespace gpproto {
    class Connection {
    public:
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void setDelegate(std::shared_ptr<ConnectionDelegate> delegate) = 0;
        virtual void sendDatas(std::list<std::shared_ptr<StreamSlice>> datas) const = 0;
        virtual void closeAndNotify() = 0;
    protected:
        bool closed;
    };
}

#endif //GPPROTO_CONNECTION_H
