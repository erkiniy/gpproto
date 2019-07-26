//
// Created by Jaloliddin Erkiniy on 8/24/18.
//

#ifndef GPPROTO_PROTODELEGATE_H
#define GPPROTO_PROTODELEGATE_H


namespace gpproto {
    class Proto;

    typedef enum {
        ProtoConnectionStateConnected = 0,
        ProtoConnectionStateConnecting = 1,
        ProtoConnectionStateWaiting = 2
    } ProtoConnectionState;

    class ProtoDelegate {
    public:
        virtual void connectionStateAvailibilityChanged(const Proto& proto, bool isNetworkAvailable) = 0;
        virtual void connectionStateChanged(const Proto& proto, ProtoConnectionState state) = 0;
    };
}

#endif //GPPROTO_PROTODELEGATE_H
