//
// Created by Jaloliddin Erkiniy on 8/24/18.
//

#ifndef GPPROTO_REQUEST_H
#define GPPROTO_REQUEST_H

#pragma once

#include <functional>
#include <string>

namespace gpproto {

    class StreamSlice;
    class RequestContext;

    using RequestCompletion = std::function<void(std::shared_ptr<StreamSlice>)>;
    using RequestFailure = std::function<void(int, std::string)>;

    class Request {
    public:

        Request(std::shared_ptr<StreamSlice> body, RequestCompletion completion = [](std::shared_ptr<StreamSlice>){}, RequestFailure failure = [](int, std::string){})
                : payload(body), completion(std::move(completion)), failure(std::move(failure)), internalId(getNextInternalId()) {};

        ~Request() = default;

        Request(const Request&) = delete;

        const int internalId;

        bool isEqual(const Request& obj) const {
            return this->internalId == obj.internalId;
        }

        std::shared_ptr<RequestContext> requestContext;

        std::shared_ptr<StreamSlice> payload;

        RequestCompletion completion;
        RequestFailure failure;

    private:
        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }

    };
}

#endif //GPPROTO_REQUEST_H
