#pragma once

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

//
//prototype
typedef NTSTATUS(NTAPI* pfnRtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS(NTAPI* pfnLdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
typedef int(WINAPI* pfnMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);


class PMSGBOX_DATA
{
public:
    pfnMessageBoxA fnMessageBoxA;

    HWND unk0;
    char chMessage[256];
    char chTitle[256];
    UINT unk1;   
};

typedef struct _THREAD_DATA
{
    pfnRtlInitUnicodeString fnRtlInitUnicodeString;
    pfnLdrLoadDll fnLdrLoadDll;
    UNICODE_STRING UnicodeString;
    WCHAR DllName[260];
    PWCHAR DllPath;
    ULONG Flags;
    HANDLE ModuleHandle;
}THREAD_DATA, * PTHREAD_DATA;