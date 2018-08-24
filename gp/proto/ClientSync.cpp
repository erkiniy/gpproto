//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "ClientSync.h"

using namespace gpproto;

gp_rx_event* ClientSync::receive(double &timeout) {
    if (currentOutputEvent)
        delete currentOutputEvent;

    currentOutputEvent = (gp_rx_event *)malloc(sizeof(gp_rx_event));

    return currentOutputEvent;
}
