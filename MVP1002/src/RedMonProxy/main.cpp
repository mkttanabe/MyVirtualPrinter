//
// main.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

#pragma comment(lib, "comctl32.lib")

#define NAME_SHAREDEVENT L"Global\\$$" APPNAME
#define TIMEOUT  60000

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	int exitcode = 0;
	WCHAR szTempPath[MAX_PATH];
	WCHAR szTempPs[MAX_PATH];

	// 引数なしの場合は スプーラサービス～RedMon 経由での起動とみなし
	// 初期プロセスを起動。UI 処理を含む子プロセスはこのプロセス内で起動する
	if (wcslen(lpCmdLine) <= 0) {
		HANDLE hEvent = INVALID_HANDLE_VALUE;
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;
		PACL  pDacl;

		// 子プロセスとの連携用イベントを作成
		// Vista 以降の session 0 分離への対応にはセキュリティデスクリプタの設定が必須
		pDacl = MySetSecurityDescriptorDacl(&sd, GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE);
		if (!pDacl) {
			dbg(L"parent: BuildRestrictedSD failed");
			exitcode = 1;
			goto EXIT_PARENT;
		}
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = FALSE;
		hEvent = CreateEvent(&sa, TRUE, TRUE, NAME_SHAREDEVENT);
		//dbg(L"parent: event=%x", hEvent);
		MyFreeDacl(pDacl);

		// PostScript用一時ファイル名を作成
		GetTempPath(sizeof(szTempPath)/sizeof(WCHAR), szTempPath);
		if (!MyGetTempFileName(szTempPath, L".ps", szTempPs)) {
			exitcode = 2;
			goto EXIT_PARENT;
		}

		// ログオンユーザのセキュリティコンテキストで本 exe の新しいプロセスを起動
		WCHAR szCmd[MAX_PATH];
		WCHAR szArg[MAX_PATH];
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		pi.hProcess = INVALID_HANDLE_VALUE;
		GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
		StringCchPrintf(szArg, ARRAY_SIZE(szArg), L" %s", szTempPs);
		DWORD sts = ExecAsUser(szCmd, szArg, &pi);
		if (sts != ERROR_SUCCESS) {
			dbg(L"parent: ExecAsUser err=%u", GetLastError());
			exitcode = 3;
			goto EXIT_PARENT;
		}
		WaitForInputIdle(pi.hProcess, TIMEOUT);

		// PostScriptを標準入力からファイルへ切り出す
		static BYTE buf[2048];
		FILE *fp;
		size_t len;
		errno_t err = _wfopen_s(&fp, szTempPs, L"wb");
		if (err != 0) {
			dbg(L"parent: open err=%d [%s]", err, szTempPs);
			exitcode = 4;
			TerminateProcess(pi.hProcess, -1);
			goto EXIT_PARENT;
		}
	    _setmode(_fileno(stdin), O_BINARY);
	    while ((len = fread(buf, 1, sizeof(buf), stdin)) > 0) {
			fwrite(buf, 1, len, fp);
	    }
	    fclose(fp);

		// 出力を完了したら子プロセスへ通知
		SetEvent(hEvent);

		// 子プロセスの完了を待って終了
		WaitForSingleObject(pi.hProcess, INFINITE);

EXIT_PARENT:
		if (GetFileAttributes(szTempPs) != 0xFFFFFFFF) {
			DeleteFile(szTempPs);
		}
		if (hEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(hEvent);
		}
		if (pi.hProcess != INVALID_HANDLE_VALUE) {
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		dbg(L"parent: exitcode=%d", exitcode);
		return exitcode;
	}

	//// 以下、子プロセス用コード ////
	WCHAR szTempPdf[MAX_PATH];
	WCHAR szOutFileName[MAX_PATH];

	InitCommonControls();
	HWND hFgWnd = GetForegroundWindow();

	// 引数で渡された PostScript 一時ファイル名を取得
	StringCchCopy(szTempPs, ARRAY_SIZE(szTempPs), __wargv[1]);

	// PostScript 一時ファイル出力の完了を待つ
	AppStartWaitMsg(NULL, L"準備中・・・");
	HANDLE hEventChild = OpenEvent(SYNCHRONIZE, TRUE, NAME_SHAREDEVENT);
	if (!hEventChild) {
		dbg(L"child: OpenEvent err=%u", GetLastError());
		exitcode = 1;
		goto EXIT_CHILD;
	}
	DWORD sts = WaitForSingleObject(hEventChild, TIMEOUT);
	if (sts != WAIT_OBJECT_0) {
		dbg(L"child: WaitForSingleObject sts=%x", sts);
		exitcode = 2;
		goto EXIT_CHILD;
	}
	AppEndWaitMsg();

	// 出力するファイル名をユーザに問い合わせる
	LPWSTR ofp = MyGetSaveFileName(
		hFgWnd,
		L"MyVirtualPrinter：出力ファイル名を指定して下さい",
		L"PDF  ファイル (*.pdf)\0*.pdf\0"   \
		L"JPEG ファイル (*.jpg)\0*.jpg\0"   \
		L"PNG  ファイル (*.png)\0*.png\0"   \
		L"GIF  ファイル (*.gif)\0*.gif\0"   \
		L"TIFF ファイル (*.tiff)\0*.tiff\0" \
		L"BMP  ファイル (*.bmp)\0*.bmp\0"   \
		L"\0",
		L"C:\\",
		L"Document");

	if (!ofp) {
		dbg(L"child: MyGetSaveFileName cancelled");
		exitcode = 0;
		goto EXIT_CHILD;
	}
	StringCchCopy(szOutFileName, ARRAY_SIZE(szOutFileName), ofp);

	AppStartWaitMsg(NULL, L"出力中：しばらくお待ち下さい・・・");

	// 指定された出力ファイルの拡張子を得る
	WCHAR *pExt = wcsrchr(szOutFileName, L'.');
	if (!pExt) {
		StringCchCat(szOutFileName, ARRAY_SIZE(szOutFileName), L".pdf"); // デフォルト
		pExt = wcsrchr(szOutFileName, L'.');
	}
	pExt++;

	// PDFの場合
	if (_wcsicmp(pExt, L"pdf") == 0) {
		if (!Ps2Pdf(szTempPs, szOutFileName, TIMEOUT)) {
			MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"PDF出力中にエラーが発生しました");
		}
	}
	// 画像形式の場合
	else if (_wcsicmp(pExt, L"jpg") == 0 ||
		     _wcsicmp(pExt, L"png") == 0 ||
			 _wcsicmp(pExt, L"gif") == 0 ||
			 _wcsicmp(pExt, L"tiff") == 0 ||
 			 _wcsicmp(pExt, L"bmp") == 0) {
		GetTempPath(sizeof(szTempPath)/sizeof(WCHAR), szTempPath);
		if (!MyGetTempFileName(szTempPath, L".pdf", szTempPdf)) {
			exitcode = 3;
			goto EXIT_CHILD;
		}
		if (!Ps2Pdf(szTempPs, szTempPdf, TIMEOUT) ||
			!Pdf2Image(szTempPdf, szOutFileName, pExt, TIMEOUT)) {
			MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"画像出力中にエラーが発生しました");
		}
		if (GetFileAttributes(szTempPdf) != 0xFFFFFFFF) {
			DeleteFile(szTempPdf);
		}
	}
	else {
		MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"その形式には対応していません");
		exitcode = 4;
		goto EXIT_CHILD;
	}

EXIT_CHILD:
	AppEndWaitMsg();
	dbg(L"child: exitcode=%d", exitcode);
	return exitcode;
}
