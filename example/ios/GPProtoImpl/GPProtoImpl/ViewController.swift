//
//  ViewController.swift
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 8/9/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    var client: UnsafeMutableRawPointer!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        self.view.backgroundColor = .red
        
        let env = gp_environment(api_id: 0x2062c46e,
                                 layer: 0,
                                 disable_updates: 0,
                                 encryption_password: strdup("testPass"),
                                 device_model: strdup("iOS Simulator C++"),
                                 system_version: strdup("12.4"),
                                 app_version: strdup("0.0.1"),
                                 lang_code: strdup("en"),
                                 documents_folder: strdup("Documents"))
        
        self.client = gp_client_create(env)
        
        //gp_client_pause(client)
        //gp_client_resume(client)
        
        let os = OutputStream.toMemory()
        os.open()
        os.writeUInt32(0x768d5f4d)
        os.writeString("998998618090")
        os.writeInt32(0)
        os.writeInt32(0)
        os.writeString("")
        os.writeString("en")
        os.close()
        
        guard let requestData = os.currentBytes else { return }
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(1)) {
            var gpData = gp_data(length: requestData.count, value: requestData.array)
            NSLog("GP_DATA \(gpData.length)")
            var txData = gp_tx_data(data: &gpData)
            let id = gp_client_send(self.client, &txData)
            NSLog("SENDING GETCODE \(id)")
        }
        
//        DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(4)) {
//            var gpData = gp_data(length: requestData.count, value: requestData.array)
//            NSLog("GP_DATA \(gpData.length)")
//            var txData = gp_tx_data(data: &gpData)
//            let id = gp_client_send(self.client, &txData)
//            NSLog("SENDING GETCODE \(id)")
//        }
    }
    
    deinit {
        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

