#pragma once

PACL MySetSecurityDescriptorDacl(IN OUT PSECURITY_DESCRIPTOR pSd, IN DWORD dwMask);

void MyFreeDacl(IN OUT PACL p);
