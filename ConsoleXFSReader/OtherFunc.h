#pragma once
#include <intrin.h>
#include <windows.h>
#include <wtypes.h>
#include <iostream>

//������� ��� ������������ ������ ����
ULONGLONG Reverse64(ULONGLONG);
DWORD Reverse32(DWORD);
WORD Reverse16(WORD);

ULONGLONG _GetFileSize(const WCHAR*);

void ShowHexData(BYTE *, ULONGLONG);


