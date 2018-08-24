//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "gp_client.h"
#include "ClientSync.h"

void *gp_client_create() {
    return new gpproto::ClientSync;
}

void gp_destroy(void *client) {
    delete static_cast<gpproto::ClientSync *>(client);
}

int gp_client_send(void *client, gp_tx_data *request) {
    if (request == nullptr || request->data == nullptr || request->data->value == nullptr)
        return 0;

    auto c = static_cast<gpproto::ClientSync *>(client);
    return c->send(request->data->value, request->data->length);
}

gp_rx_event *gp_client_receive(void *client, double timeout) {
    auto c = static_cast<gpproto::ClientSync *>(client);
    return c->receive(timeout);
}

void gp_client_pause(void *client) {

}

void gp_client_resume(void *client) {

}

void gp_client_reset_credentials(void *client) {

}