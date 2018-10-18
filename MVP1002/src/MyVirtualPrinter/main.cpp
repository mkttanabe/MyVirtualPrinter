//
// main.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// ステータスバーへメッセージを表示
void MyStatusMessage(HWND hDlg, LPCWSTR pszText)
{
	static HWND hStsBar = NULL;
	if (!hStsBar) {
		hStsBar = GetDlgItem(hDlg, IDC_STATUSBAR);
	}
	SendMessage(hStsBar, SB_SETTEXT, 0, (LPARAM)pszText);
}

// WM_INITDIALOG ハンドラ
LRESULT OnInitDialog(HWND hDlg, WPARAM wp, LPARAM lp)
{
	// Print Sppler サービスが開始されていなければ開始
	DWORD dwErr;
	DWORD ret = MyQueryServiceStatus(hDlg, L"Spooler", &dwErr);
	if (dwErr == ERROR_SUCCESS) {
		if (ret != SERVICE_RUNNING) {
			MyStartService(hDlg, L"Spooler", &dwErr);
		}
	}
	// ステータスバーを作成
	CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, L"", hDlg, IDC_STATUSBAR);

	// MyVirtualPrinter インストール状況をチェック
	int sts = IsPrinterExist(hDlg, NAME_MYPRINTER);
	if (sts == 0) {
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), FALSE);
		CheckDlgButton(hDlg, IDC_RADIO1, BST_CHECKED);
		MyStatusMessage(hDlg, L"実行ボタンを押すと登録を開始します");
	} else if (sts == 1) {
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
		CheckDlgButton(hDlg, IDC_RADIO2, BST_CHECKED);
		MyStatusMessage(hDlg, L"実行ボタンを押すと削除を開始します");
	} else {
		MB(hDlg, MB_OK|MB_ICONINFORMATION, L"未知のエラー");
		SendMessage(hDlg, WM_CLOSE, 0, 0);
	}
	return TRUE;
}

// WM_COMMAND ハンドラ
LRESULT OnCommand(HWND hDlg, WPARAM wp, LPARAM lp)
{
	HWND hOK = GetDlgItem(hDlg, IDOK);
	HWND hCANCEL = GetDlgItem(hDlg, IDCANCEL);
	HWND hR1 = GetDlgItem(hDlg, IDC_RADIO1);
	HWND hR2 = GetDlgItem(hDlg, IDC_RADIO2);
	BOOL bDoInstall;

	switch (LOWORD(wp)) {
	case IDOK:
		if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
			bDoInstall = TRUE;
		} else {
			bDoInstall = FALSE;
		}
		if (bDoInstall && IDNO == MB(hDlg, MB_YESNO|MB_ICONQUESTION,
								NAME_MYPRINTER L" をシステムに登録します。よろしいですか？")) {
			return TRUE;
		}
		if (!bDoInstall && IDNO == MB(hDlg, MB_YESNO|MB_ICONQUESTION,
								NAME_MYPRINTER L" をシステムから削除します。よろしいですか？")) {
			return TRUE;
		}
		EnableWindow(hOK, FALSE);
		EnableWindow(hCANCEL, FALSE);
		EnableWindow(hR1, FALSE);
		EnableWindow(hR2, FALSE);
		UpdateWindow(hDlg);

		if (bDoInstall) {
			MyStatusMessage(hDlg, L"ポートモニタを設定しています");
			if (!InstallMyPortMonitor(hDlg)) {
				MyStatusMessage(hDlg, L"ポートモニタ設定中にエラーが発生しました");
				goto ERR;
			}
			MyStatusMessage(hDlg, L"専用ポートを追加しています");
			if (!CreateMyPort(hDlg)) {
				MyStatusMessage(hDlg, L"ポート追加中にエラーが発生しました");
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MyStatusMessage(hDlg, L"プリンタドライバを設定しています");
			if (!InstallMyPrinterDriver(hDlg)) {
				MyStatusMessage(hDlg, L"プリンタドライバ設定中にエラーが発生しました");
				DeleteMyPort(hDlg, FALSE);
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MyStatusMessage(hDlg, L"仮想プリンタを登録しています");
			if (!CreateMyPrinter(hDlg)) {
				MyStatusMessage(hDlg, L"プリンタ登録中にエラーが発生しました");
				DeleteMyPrinterDriver(hDlg, FALSE);
				DeleteMyPort(hDlg, FALSE);
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MB(hDlg, MB_OK|MB_ICONINFORMATION, 
				L"仮想プリンタ「" NAME_MYPRINTER L"」をシステムに登録しました。"
				L"\n\n"
				NAME_MYPRINTER L" をシステムから削除するまでは、"
				L"\n\n"
				L"現在のフォルダを削除しないで下さい。");
		}
		else {
			BOOL sts = TRUE;
			DWORD err;
			MyStatusMessage(hDlg, L"スプーラサービスを再起動しています");
			MyRestartService(hDlg, L"Spooler", &err);

			MyStatusMessage(hDlg, L"仮想プリンタを削除しています");
			if (!DeleteMyPrinter(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"プリンタドライバを再設定しています");
			if (!DeleteMyPrinterDriver(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"専用ポートを削除しています");
			if (!DeleteMyPort(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"ポートモニタを再設定しています");
			if (!DeleteMyPortMonitor(hDlg, TRUE)) {
				sts = FALSE;
			}
			if (sts) {
				MB(hDlg, MB_OK|MB_ICONINFORMATION, L"仮想プリンタ「" NAME_MYPRINTER L"」をシステムから削除しました");
			}
		}
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		return TRUE;

	case IDCANCEL:
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;
	}
	return TRUE;

ERR:
	EnableWindow(hCANCEL, TRUE);
	return TRUE;
}

// ダイアログプロシージャ
LRESULT CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		return OnInitDialog(hDlg, wp, lp);

	case WM_COMMAND:
		return OnCommand(hDlg, wp, lp);

	case WM_SYSCOMMAND:
		return (wp == SC_CLOSE) ? TRUE : FALSE;

	case WM_CLOSE:
		EndDialog(hDlg, IDOK);
		return TRUE;

	default:
		return FALSE;
	}
	return FALSE;
}

// WinMain
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine, int nCmdShow)
{
	if (!IsTargetPlatform()) {
		MB(NULL, MB_OK, L"このプラットフォームには対応していません");
		return 0;
	}
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgProc);

	return 0;
}
