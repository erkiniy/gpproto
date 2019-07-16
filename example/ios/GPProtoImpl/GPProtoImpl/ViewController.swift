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
        
        client = gp_client_create()
        gp_client_pause(client)
        gp_client_resume(client)
        
    }
    
    deinit {
        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

