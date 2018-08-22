//
//  ProtoImpl.m
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 8/9/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

#include "ProtoImpl.h"
#include "gp/proto/Proto.h"

@implementation ProtoImpl
using namespace gpproto;

Proto* _proto;
std::string _address;

- (instancetype)init
{
    self = [super init];
    if (self) {
        _address = "195.158.12.163";
        _proto = new Proto(_address, 8281);
    }
    return self;
}

- (void)dealloc
{
    delete _proto;
}

- (void)initProto
{
    _proto->init();
}

- (void) start
{
    _proto->start();
}

- (void) stop
{
    _proto->stop();
}

- (void)sendData:(NSData *)data
{
    size_t size = (size_t)data.length;
    _proto->send(static_cast<unsigned const char*>(data.bytes), size);
}
@end
