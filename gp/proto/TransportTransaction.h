//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_TRANSPORTTRANSACTION_H
#define GPPROTO_TRANSPORTTRANSACTION_H

#include <memory>
#include <functional>

namespace gpproto
{
    class StreamSlice;

    class TransportTransaction {
    public:
        TransportTransaction(std::shared_ptr<StreamSlice> data, std::function<void(bool, int)> completion, bool expectsDataInResponse)
                : data(std::move(data)),
                  completion(std::move(completion)),
                  expectsDataInResponse(expectsDataInResponse) {};

        const std::shared_ptr<StreamSlice> data;
        const std::function<void(bool, int)> completion;
        const bool expectsDataInResponse;
    };
}

#endif //GPPROTO_TRANSPORTTRANSACTION_H
