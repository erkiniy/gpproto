//
//  main.swift
//  gpSwift
//
//  Created by Jaloliddin Erkiniy on 8/22/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

import Foundation

print("Hello, World!")

let impl = ProtoImpl()

DispatchQueue.global().async {
    impl?.start()
}

while (true) {
    Thread.sleep(forTimeInterval: 1.0)
}

