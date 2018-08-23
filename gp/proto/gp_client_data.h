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

struct gp_rx_event {
    enum gp_event type;
    gp_rx_data *data;
    gp_connection_state *state;
};

struct gp_tx_data {
    const gp_data *data;
};

#ifdef __cplusplus
}
#endif

#endif //GPPROTO_GP_CLIENT_DATA_H
