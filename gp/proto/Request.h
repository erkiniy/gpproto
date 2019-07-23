//
// Created by Jaloliddin Erkiniy on 8/24/18.
//

#ifndef GPPROTO_REQUEST_H
#define GPPROTO_REQUEST_H

#pragma once

#include <functional>

namespace gpproto {

    class StreamSlice;
    class RequestContext;

    using RequestCompletion = std::function<void(std::shared_ptr<StreamSlice>)> ;
    using RequestFailure = std::function<void(int, std::string)> ;

    class Request {
    public:
        explicit Request(std::shared_ptr<StreamSlice> body, RequestCompletion completion = [](std::shared_ptr<StreamSlice>){}, RequestFailure failure = [](int, std::string){})
                : internalId(getNextInternalId()), body(body), completion(std::move(completion)), failure(std::move(failure)) {};

        ~Request() = default;

        Request(const Request&) = delete;

        const int internalId;

        bool isEqual(const Request& obj) const {
            return this->internalId == obj.internalId;
        }

    protected:
        std::shared_ptr<StreamSlice> body;
        RequestCompletion completion;
        RequestFailure failure;

    private:
        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }

        std::shared_ptr<RequestContext> requestContext;
    };
}

#endif //GPPROTO_REQUEST_H
