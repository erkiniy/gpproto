//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_TIMEFIXCONTEXT_H
#define GPPROTO_TIMEFIXCONTEXT_H

#include <cstdint>

namespace gpproto {
    struct TimeFixContext {
        TimeFixContext(int64_t messageId, int32_t messageSeqNo, double timeFixAbsoluteStartTime)
                : messageId(messageId), messageSeqNo(messageSeqNo), timeFixAbsoluteStartTime(timeFixAbsoluteStartTime) {};

        const int64_t messageId;
        const int32_t messageSeqNo;
        const double timeFixAbsoluteStartTime;
    };
}
#endif //GPPROTO_TIMEFIXCONTEXT_H
