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

ClientSync::ClientSync(std::shared_ptr<gp_environment> environment) {
    auto context = std::make_shared<Context>(environment);
    context->setDatacenterSeedAddress(DatacenterAddress("", "195.158.12.163", 1112), 1);
    proto = std::make_shared<Proto>(context, 1, false);
    requestService = std::make_shared<RequestMessageService>(context);
    proto->addMessageService(requestService);

    proto->initialize();
}

void ClientSync::pause() {
    proto->pause();
}

void ClientSync::resume() {
    proto->resume();
}

void ClientSync::reset() {

}

int ClientSync::send(const unsigned char *data, size_t length) {
    auto request = std::make_shared<Request>(std::make_shared<StreamSlice>(data, length));
    int id = request->internalId;

    LOGV("Sending request with internalId=%d, size=%zu", id, length);

    request->completion = [weakSelf = weak_from_this(), id](std::shared_ptr<StreamSlice> responseData) {
        if (auto self = weakSelf.lock())
        {
            LOGV("[ClientSync] response received");

            gp_rx_data *data = new gp_rx_data;

            data->id = id;
            memcpy(data->data, responseData->begin(), responseData->size);
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
    lastReceiveEvent = pop_front();
    if (lastReceiveEvent == nullptr) return nullptr;

    return lastReceiveEvent.get();
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
