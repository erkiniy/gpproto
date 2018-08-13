//
//  ProtoImpl.h
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 8/9/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

#ifndef ProtoImpl_h
#define ProtoImpl_h
#import <Foundation/Foundation.h>

@interface ProtoImpl : NSObject

- (id)init;
- (void)dealloc;

- (void)start;
- (void)stop;

- (void)sendData:(NSData *)data;


@end

#endif /* ProtoImpl_h */
