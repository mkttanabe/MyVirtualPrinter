// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#define WIN32_LEAN_AND_MEAN	
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define _USE_32BIT_TIME_T

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <commdlg.h>
#include <commctrl.h>
#include <userenv.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <time.h>

#include "resource.h"
#include "WaitMessage.h"
#include "Uty.h"
#include "Exec.h"
#include "Sd.h"

#define ARRAY_SIZE(X) (sizeof(X)/(sizeof(X[0])))

#define APPNAME L"RedMonProxy"

