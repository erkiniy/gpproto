//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "gp/proto/gp_client.h"
#include "gp/proto/ClientSync.h"
#include "gp_client_data.h"

#include <unordered_map>

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

int gp_client_create(gp_environment environment) {
    std::unique_lock<std::mutex> lock(mutex);

    gp_environment* envPtr = (gp_environment *)malloc(sizeof(environment));
    memcpy(envPtr, &environment, sizeof(environment));

    auto client = std::make_shared<gpproto::ClientSync>(std::shared_ptr<gp_environment>(envPtr));
    client->initialize();

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

void gp_client_reset_credentials(int client) {

}