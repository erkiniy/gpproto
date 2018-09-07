//
//  main.swift
//  gpSwift
//
//  Created by Jaloliddin Erkiniy on 8/22/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

import Foundation

print("Hello, World!")

let client = gp_client_create()
gp_client_pause(client)
gp_client_resume(client)

while (true) {
    Thread.sleep(forTimeInterval: 1.0)
}

