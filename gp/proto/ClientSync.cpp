//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "ClientSync.h"
#include "gp_client_data.h"

using namespace gpproto;

gp_rx_event* ClientSync::receive(double &timeout) {
    if (currentOutputEvent)
        delete currentOutputEvent;

    return nullptr;
}
