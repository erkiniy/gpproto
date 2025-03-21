//
//  ViewController.swift
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 8/9/18.
//  Copyright © 2018 Jaloliddin Erkiniy. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    var isConnected: Bool = false;
    var client: Int32 = -1
    var stopped: Bool = false
    
    let queue = DispatchQueue(label: "uz.libproto.receive")
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .red
        
        var types: [UInt32] = [0xe317af7e, 0x727c0b9a, 0xa136b45c, 0x5bff9f3f, 0x101b9b6f, 0x6085868f, 0xd0e0f8e4, 0x517c63bf]
        
        let path = NSString(string: NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true).first!).appendingPathComponent("p_keychain")
        
        NSLog("Path is \(path)")
        
        if !FileManager.default.fileExists(atPath: path) {
            try? FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: true, attributes: nil)
        }
        
        let env = gp_environment(api_id: 0x2062c46e,
                                 layer: 0,
                                 disable_updates: 1,
                                 encryption_password: strdup("testPass"),
                                 device_model: strdup("iOS Simulator C++"),
                                 system_version: strdup("12.4"),
                                 app_version: strdup("0.0.1"),
                                 lang_code: strdup("en"),
                                 documents_folder: strdup("\(path)"),
                                 supported_types_count: Int32(types.count),
                                 supported_types: &types)
        
        self.client = gp_client_create(env)
        
//        self.queue.async {
//            while !self.stopped
//            {
//                guard let event = gp_client_receive(self.client, -1.0)?.pointee else { continue }
//                guard let data = event.data?.pointee else { continue }
//                guard let bytes = data.data?.pointee else { continue }
//
//                let nativeData = Data(bytes: bytes.value, count: bytes.length)
//
//                let iS = InputStream(data: nativeData)
//                iS.open()
//
//                let id = try! iS.readNumber(type: UInt32.self)
//                let phoneRegistered = try! iS.readBool()
//                let phoneCodeHash = try! iS.readString()
//                let sendCallTimeout = try! iS.readNumber(type: UInt32.self)
//                let isPassword = try! iS.readBool()
//
//                iS.close()
//
//                //DispatchQueue.main.async {
//                    NSLog("Response received \(id), \(phoneRegistered), \(phoneCodeHash), \(sendCallTimeout), \(isPassword)")
//                //}
//            }
//        }
        
        gp_client_pause(client)
        gp_client_resume(client)
        
//        let os = OutputStream.toMemory()
//        os.open()
//        os.writeUInt32(0x768d5f4d)
//        os.writeString("998998618090")
//        os.writeInt32(0)
//        os.writeInt32(0)
//        os.writeString("")
//        os.writeString("en")
//        os.close()
//
//        guard let requestData = os.currentBytes else { return }
//
//        DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(1)) {
//            let bytes = requestData.array
//            var gpData = gp_data(length: bytes.count, value: bytes)
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

