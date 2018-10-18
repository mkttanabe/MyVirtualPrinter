//
// Uty.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// OutputDebugString ラッパ
void dbg(LPCWSTR format, ...)
{
	WCHAR buf[2048];
	va_list args;
	va_start(args, format);
	StringCchVPrintf(buf, ARRAY_SIZE(buf), format, args);
	va_end(args);
	if (buf[wcslen(buf)-1] != L'\n') {
		StringCchCat(buf, ARRAY_SIZE(buf), L"\n");
	}
	OutputDebugString(buf);
}

// MessageBox ラッパ
int MB(HWND hParentWnd, UINT uType, LPCWSTR format, ...)
{
	WCHAR buf[2048];
	va_list args;
	va_start(args, format);
	StringCchVPrintf(buf, ARRAY_SIZE(buf), format, args);
	va_end(args);
	return MessageBox(hParentWnd, buf, APPNAME, uType);
}

// 指定された名前のプリンタがシステムに存在するか
//  0: 存在しない
//  1: 存在する
// -1: エラー
int IsPrinterExist(HWND hDlg, LPCWSTR pszPrinterName)
{
	DWORD dwSize;
    DWORD dwNeeded;
    DWORD dwReturn;
    PRINTER_INFO_5 *pPrinterInfo;

    // プリンタ列挙に必要なサイズを取得
	if (!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, NULL, 0, &dwNeeded, &dwReturn)) {
		DWORD err = GetLastError();
		if (err != ERROR_INSUFFICIENT_BUFFER) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"IsPrinterExists: EnumPrinters1 err=%u", err);
			return -1;
		}
	}	
	pPrinterInfo = (PRINTER_INFO_5 *)malloc(dwNeeded);
	// 情報取得
	dwSize = dwNeeded;
	if (!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, (LPBYTE)pPrinterInfo, dwSize, &dwNeeded, &dwReturn)) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"IsPrinterExists: EnumPrinters2 err=%u", GetLastError());
        free(pPrinterInfo);
		return -1;
	}
	if (dwReturn == 0) { // プリンタなし
		free(pPrinterInfo);
		return 0;
	}
    // 照合
    for (DWORD i = 0; i < dwReturn; i++) {
		if (_wcsicmp(pPrinterInfo[i].pPrinterName, pszPrinterName) == 0) {
			free(pPrinterInfo);
			return 1;
		}
	}
	free(pPrinterInfo);
	return 0;
}

