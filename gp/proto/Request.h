//
// Created by Jaloliddin Erkiniy on 8/24/18.
//

#ifndef GPPROTO_REQUEST_H
#define GPPROTO_REQUEST_H

#pragma once

#include <functional>
#include "gp/utils/StreamSlice.h"

namespace gpproto {

    using RequestCompletion = std::function<void(std::shared_ptr<StreamSlice>)> ;
    using RequestFailure = std::function<void(int, std::string)> ;

    class Request {
    public:
        Request(std::shared_ptr<StreamSlice> body, RequestCompletion completion = [](std::shared_ptr<StreamSlice>){}, RequestFailure failure = [](int, std::string){})
                : body(body), completion(std::move(completion)), failure(std::move(failure)) {};

        ~Request() = default;

        Request(const Request&) = delete;

    private:
        std::shared_ptr<StreamSlice> body;
        RequestCompletion completion;
        RequestFailure failure;
    };
}

#endif //GPPROTO_REQUEST_H
