//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_GP_CLIENT_H
#define GPPROTO_GP_CLIENT_H

#include <gputils/gp/utils/Common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct gp_error {
    int code;
    const char *desc;
};

struct gp_data {
    size_t length;
    const unsigned char *value;
};

struct gp_rx_data {
    int id;
    const gp_error *error;
    const gp_data *data;
};

struct gp_tx_data {
    const gp_data* data;
};

void *gp_client_create();

void gp_destroy(void *client);

int gp_client_send(void *client, gp_tx_data *request);

gp_rx_data *gp_client_receive(void *client, double timeout);

void gp_client_pause(void *client);

void gp_client_resume(void *client);

void gp_client_reset_credentials(void *client);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //GPPROTO_GP_CLIENT_H
