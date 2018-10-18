//
// Exec.cpp  プロセス処理
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"
#include <wtsapi32.h>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "userenv.lib")

#define MODNAME_GHOSTSCRIPT   L"gswin32c.exe"
#define MODNAME_IMAGEMAGICK   L"convert.exe"

#define ARGS_GHOSTSCRIPT \
	L" -q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=pdfwrite"  \
	L" -dCompatibilityLevel=1.4 -dPDFSETTING=/prepress" \
	L" -sOutputFile=\"%s\" -f \"%s\""

#define DENSITY_IMAGEMAGICK  L"120" // 出力画像のピクセル密度
#define ARGS_IMAGEMAGICK \
	L" -density " DENSITY_IMAGEMAGICK L" \"%s\" \"%s\""
#define ARGS_IMAGEMAGICK_ADJOIN \
	L" -density " DENSITY_IMAGEMAGICK L" -adjoin \"%s\" \"%s\""

// PostScript -> PDF
BOOL Ps2Pdf(IN LPCWSTR pszPSFileName, IN LPCWSTR pszPdfFileName, IN DWORD dwTimeoutMsec)
{
	WCHAR szCmd[MAX_PATH];
	WCHAR szArgs[512];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
	WCHAR *p = wcsrchr(szCmd, L'\\');
	*(p+1) = L'\0';

	StringCchCat(szCmd, ARRAY_SIZE(szCmd), MODNAME_GHOSTSCRIPT);
	StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_GHOSTSCRIPT, pszPdfFileName, pszPSFileName);

	if (!CreateProcess(szCmd, szArgs, NULL, NULL, FALSE,
					CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		return FALSE;
	}
	// 指定時間を経過してもプロセスが終了しなければ強制終了
	if (WaitForSingleObject(pi.hProcess, dwTimeoutMsec) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, -1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return FALSE;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

// PDF -> 画像形式
BOOL Pdf2Image(IN LPCWSTR pszPdfFileName, IN LPCWSTR pszImageFileName, IN LPCWSTR pszExt, IN DWORD dwTimeoutMsec)
{
	WCHAR szCmd[MAX_PATH];
	WCHAR szArgs[512];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	if (!pszPdfFileName || wcslen(pszPdfFileName) <= 0 ||
		!pszImageFileName || wcslen(pszImageFileName) <= 0) {
		return FALSE;
	}

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
	WCHAR *p = wcsrchr(szCmd, L'\\');
	*(p+1) = L'\0';

	// ImageMagickのconvert.exeはgswin32c.exeに依存する。
	// 同じフォルダ上のgswin32c.exeを参照させるために
	// カレントディレクトリを移動 -> gsを探してレジストリを見に行くのを回避
	SetCurrentDirectory(szCmd);

	StringCchCat(szCmd, ARRAY_SIZE(szCmd), MODNAME_IMAGEMAGICK);
	if (_wcsicmp(pszExt, L"tiff") == 0) {
		StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_IMAGEMAGICK, pszPdfFileName, pszImageFileName);
	} else {
		StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_IMAGEMAGICK_ADJOIN, pszPdfFileName, pszImageFileName); // 画像を個別に出力
	}
	if (!CreateProcess(szCmd, szArgs, NULL, NULL, FALSE,
				CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		return FALSE;
	}
	// 指定時間を経過してもプロセスが終了しなければ強制終了
	if (WaitForSingleObject(pi.hProcess, dwTimeoutMsec) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, -1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return FALSE;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

// 現在のログオンユーザのセキュリティコンテキストで
// 新しいプロセスを起動
// 本関数は LocalSystem Account から呼び出す必要がある
DWORD ExecAsUser(IN LPCWSTR szApp, IN LPWSTR szCmdline, IN OUT PROCESS_INFORMATION *ppi)
{
	DWORD err = ERROR_SUCCESS;
	HANDLE hToken = INVALID_HANDLE_VALUE;
	HANDLE hPrimaryToken = INVALID_HANDLE_VALUE;
	VOID *pUserEnv = NULL;
	STARTUPINFO si;

	// 現在のアクティブセッションのIDを得る
	DWORD dwSessionId = WTSGetActiveConsoleSessionId();

	// セッションIDに対応するログオンユーザのアクセストークンを得る
	if (!WTSQueryUserToken(dwSessionId, &hToken)) {
		err = GetLastError();
		dbg(L"WTSQueryUserToken err=%u session=%u", err, dwSessionId);
		goto DONE;
	}
	// CreateProcessAsUser 用にプライマリアクセストークンを得る
	if (!DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation,
								TokenPrimary, &hPrimaryToken)) {
		err = GetLastError();
		dbg(L"DuplicateTokenEx err=%u", err);
		goto DONE;
	}
	CloseHandle(hToken);

	// プライマリアクセストークンから現ユーザの環境ブロックを取得
	if (!CreateEnvironmentBlock(&pUserEnv, hPrimaryToken, FALSE)) {
		err = GetLastError();
		dbg(L"CreateEnvironmentBlock err=%u", err);
		goto DONE;
	}

	// 現ログオンユーザのセキュリティコンテキストでプロセスを起動
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(ppi, sizeof(PROCESS_INFORMATION));
	si.wShowWindow = SW_SHOW;
	si.lpDesktop = L"Winsta0\\Default"; // デフォルトのアプリケーションデスクトップ
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcessAsUser(hPrimaryToken, szApp, szCmdline,
								NULL, NULL, FALSE,
								CREATE_UNICODE_ENVIRONMENT | DETACHED_PROCESS,
								pUserEnv, // ユーザ環境ブロックを明示
								NULL, &si, ppi)) {
		err = GetLastError();
		dbg(L"CreateProcessAsUser err=%u", err);
		goto DONE;
	}

DONE:
	if (pUserEnv) {
		DestroyEnvironmentBlock(pUserEnv);
	}
	if (hToken != INVALID_HANDLE_VALUE) {
		CloseHandle(hToken);
	}
	if (hPrimaryToken != INVALID_HANDLE_VALUE) {
		CloseHandle(hPrimaryToken);
	}
    return err;
}

