//
// Created by Jaloliddin Erkiniy on 7/23/19.
//

#ifndef GPPROTO_REQUESTCONTEXT_H
#define GPPROTO_REQUESTCONTEXT_H


#include <cstdint>

namespace gpproto {
    class RequestContext {
    public:
        explicit RequestContext(int64_t messageId, int messageSeqNo, int transactionId): messageId(messageId), messageSeqNo(messageSeqNo), transactionId(transactionId) {};

        const int64_t messageId;
        const int messageSeqNo;
        bool waitingForMessageId;
        int transactionId;
        bool delivered;
        int64_t responseMessageId;
        bool willInitializeApi;
    };
}



#endif //GPPROTO_REQUESTCONTEXT_H
