#pragma once

BOOL MyGetTempFileName(
		IN LPCWSTR pszTargetDir,
		IN LPCWSTR pszExt,
		OUT WCHAR szOutFileName[MAX_PATH]);

LPWSTR MyGetSaveFileName(
		IN HWND hParentWnd,
		IN LPCWSTR pszTitle,
		IN LPCWSTR pszFilter,
		IN LPCWSTR pszInitDir,
		IN LPCWSTR pszDefaultName);

void dbg(LPCWSTR format, ...);

int MB(HWND hParentWnd, UINT uType, LPCWSTR format, ...);
