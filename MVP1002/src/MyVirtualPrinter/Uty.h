#pragma once

void dbg(LPCWSTR format, ...);
int MB(HWND hParentWnd, UINT uType, LPCWSTR format, ...);
int IsPrinterExist(HWND hDlg, LPCWSTR pszPrinterName);

