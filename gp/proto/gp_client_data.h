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
    const char *desc;
} gp_error;

typedef struct {
    size_t length;
    const unsigned char *value;
} gp_data;

struct gp_rx_data{
    int id;
    const gp_error *error;
    const gp_data *data;
};

struct gp_rx_event {
    enum gp_event type;
    gp_rx_data *data;
    enum gp_connection_state state;

    ~gp_rx_event() {
        printf("Deinitializing gp_rx_event");
        delete data;
    };
};

struct gp_tx_data {
    const gp_data *data;

    ~gp_tx_data() {
        delete data;
    }
};

struct gp_environment {
    int api_id;
    int layer;
    const bool disable_updates;
    const char *encryption_password;
    const char *device_model;
    const char *system_version;
    const char *app_version;
    const char *lang_code;
    const char *documents_folder;
};

#ifdef __cplusplus
}
#endif

#endif //GPPROTO_GP_CLIENT_DATA_H
