//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "gp/proto/ClientSync.h"
#include "gp/proto/Proto.h"
#include "gp/proto/Context.h"

using namespace gpproto;

ClientSync::ClientSync(std::shared_ptr<gp_environment> environment) {
    auto context = std::make_shared<Context>(environment);
    context->setDatacenterSeedAddress(DatacenterAddress("", "195.158.12.163", 1112), 1);
    proto = std::make_shared<Proto>(context, 1, false);
    proto->initialize();
}

void ClientSync::pause() {
    proto->pause();
}

void ClientSync::resume() {
    proto->resume();
}

void ClientSync::reset() {

}

int ClientSync::send(const unsigned char *data, size_t length) {
    return 0;
}

gp_rx_event* ClientSync::receive(double &timeout) {
    if (currentOutputEvent)
        delete currentOutputEvent;

    currentOutputEvent = (gp_rx_event *)malloc(sizeof(gp_rx_event));

    return currentOutputEvent;
}
