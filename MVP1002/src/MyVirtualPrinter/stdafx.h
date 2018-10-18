// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winspool.h>
#include <commctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include "resource.h"
#include "Platform.h"
#include "Service.h"
#include "Uty.h"
#include "Register.h"
#include "UnRegister.h"

#define ARRAY_SIZE(X) (sizeof(X)/(sizeof(X[0])))

#define APPNAME L"MyVirtualPrinter"


#define IDC_STATUSBAR 9999

#define ENVIRONMENT L"Windows NT x86"

#define NAME_REDMONDLL L"redmonnt.dll"

#define NAME_MYPRINTER       L"MyVirtualPrinter"
#define NAME_MYPRINTERDRIVER NAME_MYPRINTER L"Driver"
#define NAME_MYPORTMONITOR   NAME_MYPRINTER L" Redirected Port"
#define NAME_MYPORT          NAME_MYPRINTER L"Port:"

#define DRIVER_PSCRIPT5   L"pscript5.dll"
#define DRIVER_PSCRIPT5UI L"ps5ui.dll"
#define DRIVER_MYPPD      NAME_MYPRINTER L".ppd"

