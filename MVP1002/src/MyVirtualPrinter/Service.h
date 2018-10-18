#pragma once

DWORD MyQueryServiceStatus(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr);
BOOL MyStartService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr);
BOOL MyStopService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr);
BOOL MyRestartService(IN HWND hParentWnd, IN LPCWSTR szService, OUT DWORD *pdwErr);