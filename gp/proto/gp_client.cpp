//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "gp/proto/gp_client.h"
#include "gp/proto/ClientSync.h"
#include "gp_client_data.h"

#include <unordered_map>
#include <gputils/gp/utils/Logging.h>

static std::unordered_map<int, std::shared_ptr<gpproto::ClientSync>> clients;
static std::mutex mutex;

static std::shared_ptr<gpproto::ClientSync> clientForId(int id) {

    std::unique_lock<std::mutex> lock(mutex);

    auto it = clients.find(id);
    if (it == clients.end()) return nullptr;

    return (*it).second;
}

static void removeClient(int id) {
    std::unique_lock<std::mutex> lock(mutex);
    auto it = clients.find(id);

    if (it != clients.end()) {
        auto client = (*it).second;
        clients.erase(id);
    }
}

int gp_client_create(gp_environment *environment) {

    gp_environment* envPtr = (gp_environment *)malloc(sizeof(*environment));
    envPtr->api_id = environment->api_id;
    envPtr->layer = environment->layer;
    envPtr->disable_updates = environment->disable_updates;
    envPtr->encryption_password = environment->encryption_password;
    envPtr->device_model = environment->device_model;
    envPtr->system_version = environment->system_version;
    envPtr->app_version = environment->app_version;
    envPtr->documents_folder = environment->documents_folder;
    envPtr->encryption_password = environment->encryption_password;
    envPtr->lang_code = environment->lang_code;
    envPtr->supported_types_count = environment->supported_types_count;

    envPtr->supported_types = (unsigned int *)malloc(4 * environment->supported_types_count);
    memcpy(envPtr->supported_types, environment->supported_types, 4 * environment->supported_types_count);

    auto client = std::make_shared<gpproto::ClientSync>(std::shared_ptr<gp_environment>(envPtr));
    client->initialize();

    std::unique_lock<std::mutex> lock(mutex);

    clients[client->id] = client;

    return client->id;
}

void gp_destroy(int client) {
    if (auto clientSync = clientForId(client))
        clientSync->stop();

    removeClient(client);
}

int gp_client_send(int client, gp_tx_data *request) {
    if (request == nullptr || request->data == nullptr || request->data->value == nullptr)
        return -1;

    if (auto clientSync = clientForId(client))
        return clientSync->send(request->data->value, request->data->length);

    return -1;
}

gp_rx_event *gp_client_receive(int client, double timeout) {

    if (auto clientSync = clientForId(client))
        return clientSync->receive(timeout);

    return nullptr;
}

void gp_client_pause(int client) {
    if (auto clientSync = clientForId(client))
        clientSync->pause();
}

void gp_client_resume(int client) {
    if (auto clientSync = clientForId(client))
        clientSync->resume();
}

void gp_client_stop(int client) {
    if (auto clientSync = clientForId(client))
        clientSync->stop();
}

void gp_client_reset_credentials(int client) {

}

double gp_client_get_global_time(int client) {
    if (auto clientSync = clientForId(client)) {
        return clientSync->getGlobalTime();
    }

    return 0.0;
}

void gp_client_cancel_request(int client, int id) {
    if (auto clientSync = clientForId(client))
        clientSync->cancel(id);
}

void gp_client_set_log_level(int level) {
    gp_log_set_log_level(level);
}