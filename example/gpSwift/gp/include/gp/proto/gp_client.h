//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_GP_CLIENT_H
#define GPPROTO_GP_CLIENT_H

#include <stdio.h>
#include "gp/proto/gp_client_data.h"

#ifdef __cplusplus
extern "C" {
#endif

void *gp_client_create();

void gp_destroy(void *client);

int gp_client_send(void *client, gp_tx_data *request);

gp_rx_event *gp_client_receive(void *client, double timeout);

void gp_client_pause(void *client);

void gp_client_resume(void *client);

void gp_client_reset_credentials(void *client);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //GPPROTO_GP_CLIENT_H
