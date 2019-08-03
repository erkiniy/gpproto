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

int gp_client_create(struct gp_environment environment);

void gp_destroy(int client);

int gp_client_send(int client, struct gp_tx_data *request);

struct gp_rx_event *gp_client_receive(int client, double timeout);

void gp_client_pause(int client);

void gp_client_resume(int client);

void gp_client_reset_credentials(int client);

double gp_client_get_global_time(int client);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //GPPROTO_GP_CLIENT_H
