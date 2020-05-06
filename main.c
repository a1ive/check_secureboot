/* https://github.com/black3r/efireboot  All rights reserved*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>
#include <windows.h>
#include <winbase.h>

const char* GV_GUID = "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}";

#ifdef UNICODE
#define ___T(str) GetWChar(str)
#else
#define ___T(str) str
#endif

static void PrintError(DWORD errorCode) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	printf("Error code: %ld\n", errorCode);
	LocalFree(lpMsgBuf);
}

static const wchar_t* GetWChar(const char* c) {
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = (wchar_t *)malloc(sizeof(wchar_t)*cSize);
    //mbstowcs(wc, c, cSize);
    size_t result;
    mbstowcs_s(&result, wc, cSize, c, cSize);
    return wc;
}

static void ObtainPrivileges(LPCTSTR privilege) {
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	BOOL res;
	DWORD error;
	// Obtain required privileges
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken failed!\n");
		PrintError(GetLastError());
		exit(error);
	}

	res = LookupPrivilegeValue(NULL, privilege, &tkp.Privileges[0].Luid);
	if (!res) {
		printf("LookupPrivilegeValue failed!\n");
		PrintError(GetLastError());
		exit(error);
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	error = GetLastError();
	if (error != ERROR_SUCCESS) {
		printf("AdjustTokenPrivileges failed\n");
		PrintError(error);
		exit(error);
	}

}

int main () {
    uint8_t res = 0;
    int data_size = 0;
    ObtainPrivileges(SE_SYSTEM_ENVIRONMENT_NAME);
    data_size = GetFirmwareEnvironmentVariable(___T("SecureBoot"), ___T(GV_GUID), &res, sizeof(uint8_t));
    if (data_size) {
        printf ("SecureBoot: %s\n", res? "enabled":"disabled");
    }
    else {
        printf ("SecureBoot: unsupported platform\n");
    }
    return 0;
}
