//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_GP_CLIENT_H
#define GPPROTO_GP_CLIENT_H

#include <stdio.h>
#include "gp/proto/gp_client_data.h"

#if defined(__ANDROID__) || defined(SWIG)
#define GPCLIENT_EXPORT
#else
#include "gp/proto/gp_client_export.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

GPCLIENT_EXPORT int gp_client_create(struct gp_environment *environment);

GPCLIENT_EXPORT void gp_destroy(int client);

GPCLIENT_EXPORT int gp_client_send(int client, struct gp_tx_data *request);

GPCLIENT_EXPORT struct gp_rx_event *gp_client_receive(int client, double timeout);

GPCLIENT_EXPORT void gp_client_pause(int client);

GPCLIENT_EXPORT void gp_client_resume(int client);

GPCLIENT_EXPORT void gp_client_stop(int client);

GPCLIENT_EXPORT void gp_client_reset_credentials(int client);

GPCLIENT_EXPORT double gp_client_get_global_time(int client);

GPCLIENT_EXPORT void gp_client_cancel_request(int client, int id);

GPCLIENT_EXPORT void gp_client_set_log_level(int level);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //GPPROTO_GP_CLIENT_H
