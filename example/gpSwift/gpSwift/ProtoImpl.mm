//
//  ProtoImpl.m
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 8/9/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

#include "ProtoImpl.h"
//#include "gp/proto/gp_client.h"

@implementation ProtoImpl

void * client;

- (instancetype)init
{
    self = [super init];
    if (self) {
        //client = gp_client_create();
        
    }
    return self;
}

- (void)dealloc
{
    //gp_destroy(client);
}

- (void)initProto
{

}

- (void) start
{

}

- (void) stop
{

}

- (void)sendData:(NSData *)data
{

}
@end
