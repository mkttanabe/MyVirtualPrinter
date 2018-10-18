#pragma once

BOOL Ps2Pdf(IN LPCWSTR pszPSFileName, IN LPCWSTR pszPdfFileName, IN DWORD dwTimeoutMsec);

BOOL Pdf2Image(IN LPCWSTR pszPdfFileName, IN LPCWSTR pszImageFileName, IN LPCWSTR pszExt, IN DWORD dwTimeoutMsec);

DWORD ExecAsUser(IN LPCWSTR szApp, IN LPWSTR szCmdline, IN OUT PROCESS_INFORMATION *ppi);
