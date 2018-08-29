//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_PROTOERROR_H
#define GPPROTO_PROTOERROR_H

namespace gpproto {
    
    struct ProtoError {
        int code;
        const char *desc;
    };
    
}
#endif //GPPROTO_PROTOERROR_H
