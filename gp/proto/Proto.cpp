//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/net/TcpConnection.h"
#include "gp/net/ConnectionDelegate.h"
#include "gp/utils/Logging.h"

namespace gpproto {
    class Proto ::Impl : ConnectionDelegate {
    public:
        Impl() : connection(std::make_unique<TcpConnection>()) {

        }

        void connectionOpened(Connection& connection) {

        }

        void connectionClosed(Connection& connection) {

        }

        void connectionDidReceiveData(std::shared_ptr<StreamSlice>) {

        }

    private:
        std::unique_ptr<TcpConnection> connection;
    };

    Proto::Proto() : impl_(std::make_unique<Impl>()) {

    }

    void Proto::start() {

    }

    void Proto::send(char *data, size_t length) {

    }
}