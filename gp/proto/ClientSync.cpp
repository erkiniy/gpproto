//
// Created by Jaloliddin Erkiniy on 8/22/18.
//


#include "gp/proto/ClientSync.h"
#include "gp/proto/Proto.h"
#include "gp/proto/Context.h"
#include "gp/proto/Request.h"
#include "gp/proto/RequestMessageService.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/SecureKeychain.h"

#include "gp_client_data.h"

#include <mutex>
#include <unordered_map>

using namespace gpproto;

static std::unordered_map<std::string, std::shared_ptr<Context>> contextsByPath;
static std::mutex mtx;

static std::shared_ptr<Context> obtainContext(std::shared_ptr<gp_environment> environment) {
    std::unique_lock<std::mutex> lock(mtx);

    auto it = contextsByPath.find(environment->documents_folder);
    if (it != contextsByPath.end())
        return (*it).second;

    auto context = std::make_shared<Context>(environment);
    context->setDatacenterSeedAddress(DatacenterAddress("", "195.158.12.163", 1112), 1);
    context->setKeychain(std::make_shared<SecureKeychain>("proto", environment->documents_folder, environment->encryption_password));

    contextsByPath[environment->documents_folder] = context;

    return context;
}

ClientSync::ClientSync(std::shared_ptr<gp_environment> environment): id(getNextInternalId()) {

    auto context = obtainContext(environment);

    proto = std::make_shared<Proto>(context, 1, false);

    requestService = std::make_shared<RequestMessageService>(context);

    if (!environment->disable_updates)
        updateService = std::make_shared<UpdateMessageService>();

    proto->addMessageService(requestService);
}

void ClientSync::initialize() {
    auto self = shared_from_this();

    if (updateService != nullptr) {
        proto->addMessageService(updateService);
        updateService->setDelegate(self);
    }

    proto->setDelegate(self);
    proto->initialize();
}

void ClientSync::pause() {
    proto->pause();
    notifyReceiveQueue();
}

void ClientSync::resume() {
    proto->resume();
}

void ClientSync::stop() {
    proto->removeMessageService(requestService);
    proto->stop();
    notifyReceiveQueue();
}

void ClientSync::cancel(int requestId) {
    requestService->cancelRequest(requestId);
}

double ClientSync::getGlobalTime() {
    return proto->getGlobalTime();
}

void ClientSync::connectionStateChanged(const gpproto::Proto &proto, gpproto::ProtoConnectionState state) {
    LOGI("[ClientSync] ~> ConnectionState changed  = %d", state);

    auto eventPtr = new gp_rx_event;
    eventPtr->data = nullptr;
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

    LOGV("Sending request with internalId=%d, bytesSize = %d", id, length);

    request->completion = [weakSelf = weak_from_this(), id](std::shared_ptr<StreamSlice> responseData) {
        if (auto self = weakSelf.lock())
        {
            LOGV("[ClientSync] response received size=%zu", responseData->size);

            auto data = new gp_rx_data;

            data->id = id;

            auto bytes = (unsigned char *)malloc(responseData->size);
            memcpy(bytes, responseData->begin(), responseData->size);

            auto innerData = new gp_data;
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
            LOGE("[ClientSync] error received = %s", desc.c_str());

            auto data = new gp_rx_data;

            auto error = new gp_error;
            error->code = code;
            error->desc = strdup(desc.c_str());

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
    cleanUpEvent(lastReceiveEvent);

    currentReceiveEvent = pop_front();
    if (currentReceiveEvent == nullptr) return nullptr;

    return currentReceiveEvent.get();
}

void ClientSync::push_back(const std::shared_ptr<gp_rx_event> & event) {
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        queue.push_back(event);

        LOGV("[ClientSync] push_back queue.size = %zu", queue.size());
    }

    cond.notify_one();
}

std::shared_ptr<gp_rx_event> ClientSync::pop_front() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cond.wait(lock);

    LOGV("[ClientSync pop_front] queue.size = %zu", queue.size());

    if (queue.empty())
        return nullptr;

    auto event = queue.front();
    queue.pop_front();

    std::shared_ptr<gp_rx_event> result(std::move(event));
    return result;
}


void ClientSync::cleanUpEvent(const std::shared_ptr<gp_rx_event> event) {
    if (event == nullptr) return;

    if (auto data = event->data)
    {
        if (auto innerData = data->data) {
            delete innerData->value;
            delete innerData;
        }

        if (auto error = data->error) {
            delete error->desc;
            delete error;
        }

        delete data;
    }
}

void ClientSync::didReceiveUpdates(const std::shared_ptr<gpproto::UpdateMessageService> &service,
                                   std::shared_ptr<StreamSlice> appData, int32_t date) {

    LOGV("[ClientSync didReceiveUpdates] updatesReceived");

    gp_rx_data *data = new gp_rx_data;

    data->id = 0;

    auto bytes = (unsigned char *)malloc(appData->size);
    memcpy(bytes, appData->begin(), appData->size);

    gp_data * innerData = new gp_data;
    innerData->length = appData->size;
    innerData->value = bytes;

    data->data = innerData;

    data->error = nullptr;

    auto eventPtr = new gp_rx_event;
    eventPtr->type = UPDATE;
    eventPtr->data = data;
    eventPtr->date = (unsigned int)date;
    eventPtr->state = CONNECTED;

    auto event = std::shared_ptr<gp_rx_event>(eventPtr);
    push_back(event);
}

void ClientSync::notifyReceiveQueue() {
    auto emptyEvent = std::shared_ptr<gp_rx_event>();
    push_back(emptyEvent);
}
