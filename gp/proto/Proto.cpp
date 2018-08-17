//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/net/TcpConnection.h"
#include "gp/net/ConnectionDelegate.h"
#include "gp/utils/Logging.h"

namespace gpproto {
    class Proto ::Impl : public std::enable_shared_from_this<ConnectionDelegate>, public ConnectionDelegate {
    public:
        Impl(const std::string& address, uint16_t port) : address(IPv4Address(address)), port(port) {
            LOGV("Impl allocated");
            connection = (std::make_shared<TcpConnection>(this->address, this->port));
        }

        ~Impl() {
            LOGV("Impl deallocated");
            connection->setDelegate(nullptr);
        };

        void init() {
            connection->setDelegate(shared_from_this());
        }

        void start() {
            connection->start();
        }

        void stop() {
            connection->stop();
        }

        void connectionOpened(const Connection& connection) {
            if (didConnect)
                didConnect();
        }

        void connectionClosed(const Connection& connection) {
            if (didDisconnect)
                didDisconnect();
        }

        void connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) {
            if (didReceiveData)
                didReceiveData(slice->bytes, slice->size);
        }

        void request(const unsigned char* data, const size_t& length) {
            auto slice = std::make_shared<StreamSlice>(data, length);
            LOGV("Impl -> sending datas");
            connection->sendDatas({slice});
        }

    public:
        void (*didConnect)();
        void (*didDisconnect)();
        void (*didReceiveData)(unsigned const char*, const size_t&);

    protected:
        std::shared_ptr<TcpConnection> connection;
        IPv4Address address;
        uint16_t port;
    };

    Proto::Proto(const std::string& address, uint16_t port) : impl_(std::make_shared<Impl>(address, port)) {

    }

    Proto::~Proto() {
        LOGV("Deinitializing Proto");
    }

    void Proto::init() {
        impl_->init();
        impl_->didConnect = callbacks.didConnect;
        impl_->didDisconnect = callbacks.didDisconnect;
        impl_->didReceiveData = callbacks.didReceiveData;
    }

    void Proto::start() {
        LOGV("Starting")
        impl_->start();
    }

    void Proto::stop() {
        impl_->stop();
    }

    void Proto::send(const unsigned char *data, size_t length) {
        impl_->request(data, std::move(length));
    }
}