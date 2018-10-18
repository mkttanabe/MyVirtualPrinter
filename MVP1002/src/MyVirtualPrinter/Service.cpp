//
// Service.cpp  サービスまわり
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// 指定されたサービスを再起動
BOOL MyRestartService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr)
{
	BOOL ret = MyStopService(hParentWnd, szService, pdwErr);
	if (ret) {
		ret = MyStartService(hParentWnd, szService, pdwErr);
	}
	return ret;
}

// 指定されたサービスを開始
BOOL MyStartService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr)
{
	BOOL ret = FALSE;
    SC_HANDLE hScm = NULL, hSvc = NULL;
	DWORD err = ERROR_SUCCESS;
	SERVICE_STATUS sts;

	hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (!hScm) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStartService: OpenSCManager err=%u", err);
		goto DONE;
	}
    hSvc = OpenService(hScm, szService, SERVICE_ALL_ACCESS);
	if (!hSvc) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStartService: OpenService err=%u", err);
		goto DONE;
	}
	ret = QueryServiceStatus(hSvc, &sts);
	if (!ret) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStartService: QueryServiceStatus err=%u", err);
		goto DONE;
	}
	if (sts.dwCurrentState == SERVICE_RUNNING) {
		ret = TRUE;
		goto DONE;
	}
    ret = StartService(hSvc, 0, NULL);
	if (!ret) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStartService: StartService err=%u", err);
		goto DONE;
	}
	for (int i = 0; i < 10; i++) {
		ret = QueryServiceStatus(hSvc, &sts);
		if (ret) {
			if (sts.dwCurrentState == SERVICE_RUNNING) {
				ret = TRUE;
				goto DONE;;
			}
		} else {
			err = GetLastError();
		}
		Sleep(1000);
	}
	ret = FALSE;
DONE:
	if (pdwErr) {
		*pdwErr = err;
	}
	if (hSvc) {
		CloseServiceHandle(hSvc);
	}
	if (hScm) {
		CloseServiceHandle(hScm);
	}
	return ret;
}

// 指定されたサービスを停止
BOOL MyStopService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr)
{
	BOOL ret = FALSE;
    SC_HANDLE hScm = NULL, hSvc = NULL;
	DWORD err = ERROR_SUCCESS;
	SERVICE_STATUS sts;

	hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (!hScm) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStopService: OpenSCManager err=%u", err);
		goto DONE;
	}
    hSvc = OpenService(hScm, szService, SERVICE_ALL_ACCESS);
	if (!hSvc) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStopService: OpenService err=%u", err);
		goto DONE;
	}
	ret = QueryServiceStatus(hSvc, &sts);
	if (!ret) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStopService: QueryServiceStatus err=%u", err);
		goto DONE;
	}
	if (sts.dwCurrentState == SERVICE_STOPPED) {
		ret = TRUE;
		goto DONE;
	}
	ret = ControlService(hSvc, SERVICE_CONTROL_STOP, &sts);
	if (!ret) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyStopService: ControlService err=%u", err);
		goto DONE;
	}
	for (int i = 0; i < 10; i++) {
		ret = QueryServiceStatus(hSvc, &sts);
		if (ret) {
			if (sts.dwCurrentState == SERVICE_STOPPED) {
				ret = TRUE;
				goto DONE;;
			}
		} else {
			err = GetLastError();
		}
		Sleep(1000);
	}
	ret = FALSE;
DONE:
	if (pdwErr) {
		*pdwErr = err;
	}
	if (hSvc) {
		CloseServiceHandle(hSvc);
	}
	if (hScm) {
		CloseServiceHandle(hScm);
	}
	return ret;
}

// 指定されたサービスの現在のステータスを返す
DWORD MyQueryServiceStatus(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr)
{
	DWORD ret = 0;
	DWORD err = ERROR_SUCCESS;
	SERVICE_STATUS sts;
    SC_HANDLE hScm = NULL, hSvc = NULL;

	hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (!hScm) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyQueryServiceStatus: OpenSCManager err=%u", err);
		goto DONE;
	}
    hSvc = OpenService(hScm, szService, GENERIC_EXECUTE | GENERIC_READ);
	if (!hSvc) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyQueryServiceStatus: OpenService err=%u", err);
		goto DONE;
	}
	if (!QueryServiceStatus(hSvc, &sts)) {
		err = GetLastError();
		MB(hParentWnd, MB_OK|MB_ICONINFORMATION, L"MyQueryServiceStatus: QueryServiceStatus err=%u", err);
		goto DONE;
	}
	ret = sts.dwCurrentState;

DONE:
	if (pdwErr) {
		*pdwErr = err;
	}
	if (hSvc) {
		CloseServiceHandle(hSvc);
	}
	if (hScm) {
		CloseServiceHandle(hScm);
	}
    return ret;
}
