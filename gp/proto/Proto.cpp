//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/net/TcpConnection.h"
#include "gp/net/ConnectionDelegate.h"
#include "gp/utils/Logging.h"

namespace gpproto {
    class Proto ::Impl : std::enable_shared_from_this<ConnectionDelegate>, ConnectionDelegate {
    public:
        Impl(IPv4Address& address, uint16_t port) : address(address), port(port), connection(std::make_unique<TcpConnection>(address, port)) {
            connection->setDelegate(shared_from_this());
        }

        ~Impl() = default;

        void start() {
            connection->start();
        }

        void stop() {
            connection->stop();
        }

        void connectionOpened(Connection& connection) {
            if (didConnect)
                didConnect();
        }

        void connectionClosed(Connection& connection) {
            if (didDisconnect)
                didDisconnect();
        }

        void connectionDidReceiveData(std::shared_ptr<StreamSlice> slice) {
            if (didReceiveData)
                didReceiveData(slice->bytes, slice->size);
        }

        void request(const char* data, size_t&& length) {
            auto slice = std::make_shared<StreamSlice>(data, length);
            connection->sendDatas({slice});
        }

    public:
        void (*didConnect)();
        void (*didDisconnect)();
        void (*didReceiveData)(const char*, const size_t&);

    protected:
        std::unique_ptr<TcpConnection> connection;
        IPv4Address address;
        uint16_t port;
    };

    Proto::Proto(IPv4Address& address, uint16_t port) : impl_(std::make_unique<Impl>(address, port)) {

    }

    Proto::~Proto() {
        LOGV("Deinitializing Proto");
    }

    void Proto::init() {
        impl_->didConnect = callbacks.didConnect;
        impl_->didDisconnect = callbacks.didDisconnect;
        impl_->didReceiveData = callbacks.didReceiveData;
    }

    void Proto::start() {
        impl_->start();
    }

    void Proto::stop() {
        impl_->stop();
    }

    void Proto::send(const char *data, size_t& length) {
        impl_->request(data, std::move(length));
    }
}