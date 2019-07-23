// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: 在此处引用程序需要的其他头文件
#include <direct.h>
#include <io.h>
#include <errno.h>
//the head file winsock2.h must before at file windows.h
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <string.h>
#include <windows.h>
#include <process.h>
#include <time.h>

#define SERVER_PORT 10010
#define LOG_SIZE_4MB 1048576
#define BUF_SIZE 1024
#define PATH_SIZE 512
#define NAME_SIZE 32
#define PWD_SIZE 64
#define MAC_SIZE 16
#define CMD_MIN_SIZE 8
#define CMD_SIZE 256
#define RX_SIZE 512
#define USR_MAX_DEV 256

#define USERS_PATH ".\\Users"
#define DEVICES_PATH ".\\Devices"

#include "Util.h"
#include "Base.h"
#include "MemLeak.h"
#include "Peer.h"
#include "Queue.h"
#include "Log.h"
#include "Device.h"
#include "DevMgr.h"
#include "User.h"
#include "UserMgr.h"
#include "UdpServer.h"
#include "Protocol.h"