//
// Platform.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// 対象プラットフォーム判定
BOOL IsTargetPlatform()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	OSVERSIONINFO version;
	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);

	DWORD id    = version.dwPlatformId;
	DWORD major = version.dwMajorVersion;
	DWORD minor = version.dwMinorVersion;

	if (id == VER_PLATFORM_WIN32_NT) {	// NTファミリ
		if (major <= 4) {
			return FALSE;
		}
		if (major == 5 && minor == 0) { // 2000
			return FALSE;
		}
		if ((major >= 5 && minor >= 1) || // 5.1=XP(32), 5.2=2003 or XP(64)
			(major >= 6)) { // Vista, 7 or later

			LPFN_ISWOW64PROCESS pIsWow64Process;
			pIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
						GetModuleHandle(L"kernel32"), "IsWow64Process");
			if (!pIsWow64Process) {
				return TRUE; // !64bit
			}
			BOOL bWow64Process = FALSE;
			pIsWow64Process(GetCurrentProcess(), &bWow64Process);
			if (bWow64Process) {
				return FALSE;
			}
			else {
				return TRUE;
			}
		}
	}
	return FALSE;		
}

// プラットフォーム種別
// 0: NG
// 1: 2000
// 2: XP
// 3: Vista
// 4: 7 or later
int GetPlatform()
{
	OSVERSIONINFO version;
	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);
	DWORD id    = version.dwPlatformId;
	DWORD major = version.dwMajorVersion;
	DWORD minor = version.dwMinorVersion;
	if (id == VER_PLATFORM_WIN32_NT) {
		if (major <= 4) {
			return 0;
		}
		if (major == 5 && minor == 0) {
			return 1;
		}
		if (major == 5 && minor >= 1) {
			return 2;
		}
		if (major == 6) {
			if (minor == 0) {
				return 3;
			} else {
				return 4;
			}
		}
		return 4;
	}
	return 0;
}

