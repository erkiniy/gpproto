//
//  Extensions.swift
//  GPProtoImpl
//
//  Created by Jaloliddin Erkiniy on 7/25/19.
//  Copyright © 2019 Jaloliddin Erkiniy. All rights reserved.
//

import Foundation
public extension BinaryInteger {
    
    public var asData: Data {
        var value = self
        return Data(bytes: &value, count: MemoryLayout.size(ofValue: self))
    }
    
    public var toByteArray: [UInt8] {
        var value = self
        return withUnsafePointer(to: &value) {
            $0.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: self)) {
                Array(UnsafeBufferPointer(start: $0, count: MemoryLayout.size(ofValue: self)))
            }
        }
    }
}

public extension Float {
    public var asData: Data {
        var value = self
        return Data(bytes: &value, count: MemoryLayout.size(ofValue: self))
    }
    
    public var toByteArray: [UInt8] {
        var value = self
        return withUnsafePointer(to: &value) {
            $0.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: self)) {
                Array(UnsafeBufferPointer(start: $0, count: MemoryLayout.size(ofValue: self)))
            }
        }
    }
}

public extension Double {
    
    public var asData: Data {
        var value = self
        return Data(bytes: &value, count: MemoryLayout.size(ofValue: self))
    }
    
    public var toByteArray: [UInt8] {
        var value = self
        return withUnsafePointer(to: &value) {
            $0.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: self)) {
                Array(UnsafeBufferPointer(start: $0, count: MemoryLayout.size(ofValue: self)))
            }
        }
    }
}

public extension UInt32 {
    public var asByteArray: [UInt8] {
        return [0, 8, 16, 24].map { UInt8(self >> $0 & 0x000000ff) }
    }
}

public extension Data {
    
    public func copyBytes(to: inout UInt64, count: Int) {
        guard self.count >= count else { return }
        let bytes: [UInt8] = self.array
        let value = UnsafePointer(bytes).withMemoryRebound(to: UInt64.self, capacity: 1) {
            $0.pointee
        }
        to = value
    }
    
    public func copyBytes(to: inout UInt32, count: Int) {
        
        guard self.count >= count else { return }
        to = UInt32(self.getInt32())
    }
    
    public func copyBytes(to: inout Int32, count: Int) {
        guard self.count >= count else { return }
        let bytes: [UInt8] = self.array
        let value = UnsafePointer(bytes).withMemoryRebound(to: Int32.self, capacity: 1) {
            $0.pointee
        }
        to = value
    }
    
    public var array: [UInt8] {
        return [UInt8](self)
    }
    
    public var hexEncodedString: String {
        let array = map { String(format: "%02hhx", $0) }
        var result: String = ""
        
        for i in 0 ..< array.count {
            if i % 4 == 0 && i != 0 {
                result += " "
            }
            result += array[i]
        }
        return result
    }
    
    public var hexEncodedStringWithoutSpaces: String {
        let array = map { String(format: "%02hhx", $0) }
        var result: String = ""
        
        for i in 0 ..< array.count {
            result += array[i]
        }
        return result
    }
    
    public func getInt32() -> Int32 {
        let iS = InputStream(data: self)
        iS.open()
        let value = iS.readInt32()
        iS.close()
        return value
    }
    
    public func getInt64() -> Int64 {
        let iS = InputStream(data: self)
        iS.open()
        let value = iS.readInt64()
        iS.close()
        return value
    }
    
    public var lower16Bytes: Data {
        guard self.count >= 16 else { return Data() }
        return self.subdata(in: self.count - 16 ..< self.count)
    }
    
    public var lower4Bytes: Data {
        guard self.count >= 4 else { return Data() }
        return self.subdata(in: self.count - 4 ..< self.count)
    }
    
    
    public func paddedData(to: Int) -> Data {
        
        guard to > 0 else { return self }
        var data = self
        var randomBuffer: [UInt8] = []
        arc4random_buf(&randomBuffer, to - 1)
        
        var take: Int = 0
        while (data.count % to != 0) {
            data.append(&randomBuffer + take, count: 1)
            take += 1
        }
        return data
    }
}

public extension InputStream {
    public func readInt32() -> Int32 {
        guard self.hasBytesAvailable else { return 0 }
        
        let length = 4
        
        let pointer = UnsafeMutablePointer<UInt8>.allocate(capacity: length)
        
        let lengthRead = self.read(pointer, maxLength: length)
        
        var value: Int32 = 0
        
        for i in 0 ..< lengthRead
        {
            let byte = pointer.advanced(by: lengthRead - 1 - i).pointee
            value <<= 8
            value |= Int32(byte)
        }
        
        pointer.deallocate(capacity: lengthRead)
        
        return value
    }
    
    public func readInt64() -> Int64 {
        guard self.hasBytesAvailable else { return 0 }
        
        let length = 8
        let pointer = UnsafeMutablePointer<UInt8>.allocate(capacity: length)
        
        let lengthRead = self.read(pointer, maxLength: length)
        var value: Int64 = 0
        
        for i in 0 ..< lengthRead
        {
            let byte = pointer.advanced(by: lengthRead - 1 - i).pointee
            value <<= 8
            value |= Int64(byte)
        }
        
        pointer.deallocate(capacity: lengthRead)
        return value
    }
}

public extension InputStream {
    
    public enum InputStreamError: Error {
        case readFailed(type: String)
    }
    
    public func readIntOrDouble<T>(type: T.Type) throws -> T {
        return try readNumber(type: type)
    }
    
    public func readNumber<T>(type: T.Type) throws -> T {
        let size: Int = MemoryLayout<T>.size
        
        let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: size)
        defer { buffer.deallocate() }
        if self.read(buffer, maxLength: size) != size {
            throw InputStreamError.readFailed(type: "\(type)")
        }
        let value = buffer.withMemoryRebound(to: T.self, capacity: size) { $0.pointee }
        return value
    }
    
    public func readData(length: Int) throws -> Data {
        guard length >= 0 && length < Int(Int32.max) else { throw InputStreamError.readFailed(type: "Data") }
        
        let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: length)
        if self.read(buffer, maxLength: length) != length {
            throw InputStreamError.readFailed(type: "Data")
        }
        return Data(bytesNoCopy: buffer, count: length, deallocator: .free)
    }
    
    public func readData(maxLength: Int) throws -> Data {
        guard maxLength >= 0 && maxLength < Int(Int32.max) else { throw InputStreamError.readFailed(type: "Data") }
        
        let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: maxLength)
        let bytesRead = self.read(buffer, maxLength: maxLength)
        guard bytesRead <= maxLength else {
            throw InputStreamError.readFailed(type: "Data")
        }
        return Data(bytesNoCopy: buffer, count: bytesRead, deallocator: .free)
    }
    
    public func readString() throws -> String {
        let bytes = try self.readBytes()
        
        if let string = String(data: bytes, encoding: .utf8) {
            return string
        }
        
        throw InputStreamError.readFailed(type: "Cannot encode to utf-8 string")
    }
    
    public func readStringRaw() throws -> String {
        let length: Int = try Int(self.readNumber(type: Int32.self))
        guard length > 0 else { return "" }
        return try String(data: self.readData(length: length), encoding: .utf8) ?? ""
    }
    
    public func readBytes() throws -> Data {
        
        let lengthMarker = try self.readNumber(type: UInt8.self)
        
        var paddingBytes: Int = 0
        
        var length: Int = 0
        if lengthMarker < 254 {
            
            length = Int(lengthMarker)
            paddingBytes = InputStream.roundUpInput(numToRound: length + 1, multiple: 4) - (length + 1)
        }
        else {
            let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 3)
            defer { buffer.deallocate() }
            
            if self.read(buffer, maxLength: 3) != 3 {
                throw InputStreamError.readFailed(type: "Bytes long length error, not 3 bytes")
            }
            
            let longLengthMarker: Int32 = buffer.withMemoryRebound(to: Int32.self, capacity: 1) { $0.pointee } & 0x00ffffff
            length = Int(longLengthMarker)
            
            paddingBytes = InputStream.roundUpInput(numToRound: Int(length), multiple: 4) - Int(length)
        }
        
        let bytes = try self.readData(length: length)
        if paddingBytes > 0 {
            var temp: UInt8 = 0
            for _ in 0 ..< paddingBytes {
                self.read(&temp, maxLength: 1)
            }
        }
        
        return bytes
    }
    
    public func readBool() throws -> Bool {
        let boolMarker: UInt32 = try self.readNumber(type: UInt32.self)
        
        let boolTrue: UInt32 = 0x997275b5
        let boolFalse: UInt32 = 0xbc799737
        
        if boolMarker == boolTrue {
            return true
        }
        
        if boolMarker == boolFalse {
            return false
        }
        
        throw InputStreamError.readFailed(type: "Bool read failed \(boolMarker)")
    }
    
    public class func roundUpInput(numToRound: Int, multiple: Int) -> Int {
        if (multiple == 0) {
            return numToRound
        }
        
        let remainder: Int = numToRound % multiple;
        if (remainder == 0) {
            return numToRound
        }
        
        return numToRound + multiple - remainder;
    }
}

public extension OutputStream {
    
    public var currentBytes: Data? {
        return self.property(forKey: .dataWrittenToMemoryStreamKey) as? Data
    }
    
    public func writeUInt8(_ value: UInt8) {
        self.write(value.toByteArray, maxLength: 1)
    }
    
    public func writeInt8(_ value: Int8) {
        self.write(value.toByteArray, maxLength: 1)
    }
    
    public func writeInt16(_ value: Int16) {
        self.write(value.toByteArray, maxLength: 2)
    }
    
    public func writeUInt16(_ value: UInt16) {
        self.write(value.toByteArray, maxLength: 2)
    }
    
    public func writeUInt32(_ value: UInt32) {
        self.write(value.toByteArray, maxLength: 4)
    }
    
    public func writeInt32(_ value: Int32) {
        self.write(value.toByteArray, maxLength: 4)
    }
    
    public func writeInt64(_ value: Int64) {
        self.write(value.toByteArray, maxLength: 8)
    }
    
    public func writeDouble(_ value: Double) {
        self.write(value.toByteArray, maxLength: 8)
    }
    public func writeFloat(_ value: Float) {
        self.write(value.toByteArray, maxLength: MemoryLayout.size(ofValue: value))
    }
    
    public func writeData(_ value: Data) {
        self.write(value.array, maxLength: value.count)
    }
    
    public func writeString(_ value: String) {
        guard let bytes: Data = value.data(using: .utf8) else {
            self.writeInt32(0)
            return
        }
        self.writeBytes(bytes)
    }
    
    public func writeStringRaw(_ value: String) {
        guard let data: Data = value.data(using: .utf8) else {
            self.writeInt32(0)
            return
        }
        let length: Int32 = Int32(data.count)
        self.writeInt32(length)
        if length != 0 {
            self.writeData(data)
        }
    }
    
    public func writeBytes(_ value: Data) {
        let length = value.count
        
        guard length > 0 else {
            
            self.writeInt32(0)
            return
        }
        
        var paddingBytes: Int = 0
        
        if length < 254 {
            self.writeUInt8(UInt8(length))
            paddingBytes = OutputStream.roundUp(numToRound: length + 1, multiple: 4) - (length + 1)
        }
        else {
            let marker: UInt8 = 254
            self.write(marker.toByteArray, maxLength: 1)
            self.write(length.toByteArray, maxLength: 3)
            paddingBytes = OutputStream.roundUp(numToRound: length, multiple: 4) - length
        }
        self.writeData(value)
        
        let zero: Int8 = 0
        
        if paddingBytes > 0 {
            for _ in 0 ..< paddingBytes {
                self.writeInt8(zero)
            }
        }
    }
    
    public func writeBool(_ value: Bool) {
        let boolMarker: UInt32 = value ? 0x997275b5 : 0xbc799737
        self.writeUInt32(boolMarker)
    }
    
    public static func roundUp(numToRound: Int, multiple: Int) -> Int
    {
        return multiple == 0 ? numToRound : ((numToRound % multiple) == 0 ? numToRound : (numToRound + multiple - (numToRound % multiple)))
    }
}
