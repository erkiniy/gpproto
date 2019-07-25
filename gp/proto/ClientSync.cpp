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
    auto request = std::make_shared<Request>(std::make_shared<StreamSlice>(data, length),
            [weakSelf = weak_from_this()](auto responseData) {
        if (auto self = weakSelf.lock())
        {

        }
        }, [weakSelf = weak_from_this()](int code, std::string desc) {
        if (auto self = weakSelf.lock())
        {

        }
    });
    return request->internalId;
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
