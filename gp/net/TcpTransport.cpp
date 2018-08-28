//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#include "TcpTransport.h"

using namespace gpproto;

void TcpTransport::reset() {

}

void TcpTransport::stop() {

}

void TcpTransport::setDelegate(std::shared_ptr<TransportDelegate> delegate) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&, delegate, strongSelf] {
        strongSelf->delegate = delegate;
    });
}

void TcpTransport::setDelegateNeedsTransaction() {

}
