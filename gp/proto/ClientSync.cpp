//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "gp/proto/ClientSync.h"
#include "gp/proto/Proto.h"
#include "gp/proto/Context.h"
#include "gp_client_data.h"
#include "gp/proto/Request.h"
#include "gp/proto/RequestMessageService.h"
#include "gp/utils/StreamSlice.h"

using namespace gpproto;

ClientSync::ClientSync(std::shared_ptr<gp_environment> environment): id(getNextInternalId()) {
    auto context = std::make_shared<Context>(environment);

    context->setDatacenterSeedAddress(DatacenterAddress("", "195.158.12.163", 1112), 1);

    proto = std::make_shared<Proto>(context, 1, false);

    requestService = std::make_shared<RequestMessageService>(context);

    proto->addMessageService(requestService);
}

ClientSync::~ClientSync() {

}

void ClientSync::initialize() {
    proto->setDelegate(shared_from_this());
    proto->initialize();
}

void ClientSync::pause() {
    proto->pause();
}

void ClientSync::resume() {
    proto->resume();
}

void ClientSync::stop() {
    proto->removeMessageService(requestService);
    proto->stop();
}

void ClientSync::connectionStateChanged(const gpproto::Proto &proto, gpproto::ProtoConnectionState state) {
    LOGI("[ClientSync] ~> ConnectionState changed  = %d", state);

    auto eventPtr = new gp_rx_event;
    eventPtr->type = PROTO_STATE;
    eventPtr->state = state == ProtoConnectionState::ProtoConnectionStateConnected ? CONNECTED : CONNECTING;

    auto event = std::shared_ptr<gp_rx_event>(eventPtr);
    push_back(event);
}

void ClientSync::connectionStateAvailibilityChanged(const gpproto::Proto &proto, bool isNetworkAvailable) {
    LOGI("[ClientSync] ~> ConnectionStateAvailibilityChanged changed  = %d", isNetworkAvailable);
}

int ClientSync::send(const unsigned char *data, size_t length) {
    auto request = std::make_shared<Request>(std::make_shared<StreamSlice>(data, length));
    int id = request->internalId;

    LOGV("Sending request with internalId=%d, bytes = %s", id, data);

    request->completion = [weakSelf = weak_from_this(), id](std::shared_ptr<StreamSlice> responseData) {
        if (auto self = weakSelf.lock())
        {
            LOGV("[ClientSync] response received %s", responseData->description().c_str());

            gp_rx_data *data = new gp_rx_data;

            data->id = id;

            auto bytes = (unsigned char *)malloc(responseData->size);
            memcpy(bytes, responseData->begin(), responseData->size);

            gp_data * innerData = new gp_data;
            innerData->length = responseData->size;
            innerData->value = bytes;

            data->data = innerData;

            data->error = nullptr;

            auto eventPtr = new gp_rx_event;
            eventPtr->type = RESPONSE;
            eventPtr->data = data;
            eventPtr->state = CONNECTED;

            auto event = std::shared_ptr<gp_rx_event>(eventPtr);
            self->push_back(event);
        }
    };

    request->failure = [weakSelf = weak_from_this(), id](int code, std::string desc) {
        if (auto self = weakSelf.lock())
        {
            LOGE("[ClientSync] error received");

            gp_rx_data *data = new gp_rx_data;

            gp_error* error = new gp_error;
            error->code = code;
            strcpy(error->desc, desc.c_str());

            data->id = id;
            data->data = nullptr;
            data->error = error;

            auto eventPtr = new gp_rx_event;
            eventPtr->type = RESPONSE;
            eventPtr->data = data;
            eventPtr->state = CONNECTED;

            auto event = std::shared_ptr<gp_rx_event>(eventPtr);
            self->push_back(event);
        }
    };

    requestService->addRequest(request);

    return id;
}

gp_rx_event* ClientSync::receive(double & timeout) {
    auto lastReceiveEvent = currentReceiveEvent;
    cleanUpEvent(std::move(lastReceiveEvent));

    currentReceiveEvent = pop_front();
    if (currentReceiveEvent == nullptr) return nullptr;

    return currentReceiveEvent.get();
}

void ClientSync::push_back(const std::shared_ptr<gp_rx_event> & event) {
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        queue.push_back(event);
    }

    cond.notify_one();
}

std::shared_ptr<gp_rx_event> ClientSync::pop_front() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cond.wait(lock);

    if (queue.empty())
        return nullptr;

    std::shared_ptr<gp_rx_event> result(std::move(queue.front()));
    return result;
}


void ClientSync::cleanUpEvent(const std::shared_ptr<gp_rx_event> && event) {
    if (event == nullptr) return;

    if (auto data = event->data)
    {
        if (auto innerData = data->data)
            delete innerData->value;

        if (auto error = data->error)
            delete error->desc;
    }

    delete event->data;
}
