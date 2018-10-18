//
// Sd.cpp  セキュリティディスクリプタの操作
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// セキュリティディスクリプタに Everyone への許可 ACE をセットし
// 生成した DACL へのポインタを返す
PACL MySetSecurityDescriptorDacl(IN OUT PSECURITY_DESCRIPTOR pSd, IN DWORD dwMask) {

	PSID pSid = NULL;
	PACL pDacl = NULL;
	DWORD dwLen;
	// SECURITY_WORLD_RID に対応
	SID_IDENTIFIER_AUTHORITY IdAuthority = SECURITY_WORLD_SID_AUTHORITY;
	BOOL bSts = FALSE;

	// セキュリティディスクリプタ初期化
	if (!InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION)) {
		dbg(L"MySetSecurityDescriptorDacl: InitializeSecurityDescriptor err=%u", GetLastError());
		goto DONE;
	}
	// Everyone 副権限つきの SID を取得
	if (!AllocateAndInitializeSid(&IdAuthority, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSid)) {
		dbg(L"MySetSecurityDescriptorDacl: AllocateAndInitializeSid err=%u",GetLastError());
		goto DONE;
	}
	// DACL の生成と初期化
	dwLen = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSid);
	pDacl = (PACL)GlobalAlloc(GPTR, dwLen);
	if (!pDacl) {
		dbg(L"MySetSecurityDescriptorDacl: GlobalAlloc err=%u", GetLastError());
		goto DONE;
	}
	if (!InitializeAcl(pDacl, dwLen, ACL_REVISION)) {
		dbg(L"MySetSecurityDescriptorDacl: InitializeAcl err=%u", GetLastError());
		goto DONE;
	}
	// Everyone への許可 ACE を DACL へ追加
	if (!AddAccessAllowedAce(pDacl, ACL_REVISION, dwMask, pSid)) {
		dbg(L"MySetSecurityDescriptorDacl: AddAccessAllowedAce err=%u", GetLastError());
		goto DONE;
	}
	// 作成した DACL をセキュリティディスクリプタにセット
	if (!SetSecurityDescriptorDacl(pSd, TRUE, pDacl, FALSE)) {
		dbg(L"MySetSecurityDescriptorDacl: SetSecurityDescriptorDacl err=%u", GetLastError());
		goto DONE;
	}
	bSts = TRUE;

DONE:
	if (pSid) {
		FreeSid(pSid);
	}
	if (!bSts) { // 何らかのエラー
		if (pDacl) {
			GlobalFree(pDacl);
		}
		return NULL;
	}
	return pDacl;
}

// MySetSecurityDescriptorDacl() 返値の DACL を開放
void MyFreeDacl(IN OUT PACL pDacl)
{
	if (pDacl) {
		GlobalFree(pDacl);
	}
}
