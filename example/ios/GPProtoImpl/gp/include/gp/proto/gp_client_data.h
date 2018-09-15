//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_GP_CLIENT_DATA_H
#define GPPROTO_GP_CLIENT_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

enum gp_event {
    RESPONSE = 1,
    UPDATE = 2,
    CONNECTION_STATE = 3
};

enum gp_connection_state {
    CONNECTING = 1,
    CONNECTED = 2
};

typedef struct  {
    int code;
    const char *desc;
} gp_error;

typedef struct {
    size_t length;
    const unsigned char *value;
} gp_data;

typedef struct {
    int id;
    const gp_error *error;
    const gp_data *data;
} gp_rx_data;

typedef struct {
    enum gp_event type;
    gp_rx_data *data;
    enum gp_connection_state *state;
} gp_rx_event;

typedef struct {
    const gp_data *data;
} gp_tx_data;

typedef struct {
    char *encryption_password;
    char *device_model;
    char *system_version;
    char *app_version;
    char *lang_code;
} gp_environment;

#ifdef __cplusplus
}
#endif

#endif //GPPROTO_GP_CLIENT_DATA_H
