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
    PROTO_STATE = 3
};

enum gp_connection_state {
    CONNECTING = 1,
    CONNECTED = 2
};

typedef struct  {
    int code;
    char *desc;
} gp_error;

struct gp_data {
    size_t length;
    const unsigned char *value;
};

struct gp_rx_data {
    int id;
    gp_error *error;
    struct gp_data *data;
};

struct gp_rx_event {
    enum gp_event type;
    struct gp_rx_data *data;
    unsigned int date;
    enum gp_connection_state state;
};

struct gp_tx_data {
    const struct gp_data *data;
};

struct gp_environment {
    int api_id;
    int layer;
    int disable_updates;
    char *encryption_password;
    char *device_model;
    char *system_version;
    char *app_version;
    char *lang_code;
    char *documents_folder;

    int supported_types_count;
    long *supported_types;
};

#ifdef __cplusplus
}
#endif

#endif //GPPROTO_GP_CLIENT_DATA_H
