//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#ifndef GPPROTO_MESSAGETRANSACTION_H
#define GPPROTO_MESSAGETRANSACTION_H

#include <vector>
#include "gp/network/OutgoingMessage.h"

namespace gpproto {
    class MessageTransaction {
    public:
        explicit MessageTransaction(std::vector<OutgoingMessage> payloads) : payloads(std::move(payloads)) {};
        const std::vector<OutgoingMessage> payloads;
    };
}
#endif //GPPROTO_MESSAGETRANSACTION_H
