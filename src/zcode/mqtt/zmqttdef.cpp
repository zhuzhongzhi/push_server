//
//  zmqttdef.cpp
//  pusher
//
//  Created by zzz on 16/4/15.
//
//

#include "zmqttdef.h"

const uchar Connection_Accepted = 0x00;
const uchar Unacceptable_Protocol_Version = 0x01;
const uchar Identifier_Rejected = 0x02;
const uchar Server_Unavailable = 0x03;
const uchar Bad_UserName_OR_Password = 0x04;
const uchar Not_Authorized = 0x05;
const uchar Reserved = 0x06; /**6-255 Reserved for future use**/

const STLString  DeviceType_IPHONE("iphone");
const STLString  DeviceType_IPAD("ipad");
const STLString  DeviceType_APHONE("aphone");
const STLString  DeviceType_APAD("apad");
const STLString  DeviceType_WPHONE("wphone");
const STLString  DeviceType_WPAD("wpad");
const STLString  DeviceType_IPC("imac");
const STLString  DeviceType_WPC("winpc");
