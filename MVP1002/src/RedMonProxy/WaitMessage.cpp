//
// WaitMessage.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

static HWND g_hWaitDlg = NULL;
static HANDLE g_hThread = NULL;
static WCHAR g_szText[128] = L"\0";

#define TIMER1 0x1001

static BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND hMessage  = GetDlgItem(hDlg, IDC_MESSAGE);
	HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS1);

	switch (msg) {

	case WM_INITDIALOG:
		g_hWaitDlg = hDlg;
		SetWindowPos(hDlg, HWND_NOTOPMOST , 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		if (*g_szText) {
			SetWindowTextW(hMessage, g_szText);
		}
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));
		SendMessage(hProgress, PBM_SETSTEP, 10, 0);
		SendMessage(hProgress, PBM_SETPOS, 0, 0);
		SetTimer(hDlg, TIMER1, 200, NULL);
		return TRUE;

	case WM_TIMER:
		SendMessage(hProgress, PBM_STEPIT, 0, 0);
		return TRUE;

	case WM_CLOSE:
		KillTimer(hDlg, TIMER1); 
		EndDialog(hDlg, IDOK);
		g_hWaitDlg = NULL;
		return TRUE;
	}
	return FALSE;
}

static unsigned int __stdcall _th(void *p)
{
	HWND hParentWnd = (HWND)p;
	DialogBox((HINSTANCE)GetModuleHandle(NULL), 
		MAKEINTRESOURCEW(IDD_WAITMSG), hParentWnd, (DLGPROC)DlgProc);
	_endthreadex(0);
	return 0;
}

void AppStartWaitMsg(HWND hParentWnd, LPCWSTR pszMsg)
{
	if (!g_hThread) {
		unsigned int tid;
		size_t len;
		*g_szText = '\0';
		if (pszMsg && (len = wcslen(pszMsg)) > 0) {
			if (len < sizeof(g_szText)/sizeof(WCHAR)) {
				StringCchCopy(g_szText, ARRAY_SIZE(g_szText), pszMsg);
			}
		}
		g_hThread = (HANDLE)_beginthreadex(NULL, 0, _th, (void*)hParentWnd, 0, &tid);
	}
}

void AppEndWaitMsg()
{
	if (g_hThread) {
		SendMessage(g_hWaitDlg, WM_CLOSE, 0, 0);
		if (WaitForSingleObject(g_hThread, 3000) == WAIT_TIMEOUT) {
			TerminateThread(g_hThread, -1);
		}
		CloseHandle(g_hThread);
		g_hThread = NULL;
	}
}

