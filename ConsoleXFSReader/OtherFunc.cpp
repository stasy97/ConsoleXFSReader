#include "OtherFunc.h"

ULONGLONG Reverse64(ULONGLONG enterValue) {
	return _byteswap_uint64(enterValue);
}

DWORD Reverse32(DWORD enterValue) {

	return _byteswap_ulong(enterValue);
}

WORD Reverse16(WORD enterValue) {

	return _byteswap_ushort(enterValue);
}

ULONGLONG _GetFileSize(const WCHAR* filename) {
	
	HANDLE fileHandle = CreateFileW(
		filename,
		FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (fileHandle != INVALID_HANDLE_VALUE) {

		LARGE_INTEGER fileSize = { 0 };

		bool result = GetFileSizeEx(fileHandle, &fileSize);
		CloseHandle(fileHandle);

		//std::cout << "lpFileSize = " << fileSize.QuadPart;
		return fileSize.QuadPart;
		
	}
	else return 0;
}

void ShowHexData(BYTE * buffer, ULONGLONG bufferSize)
{
	for (int i = 1; i < bufferSize + 1; i++) {
		printf("%02x ", buffer[i - 1]);

		if (i % 16 == 0) {
			std::cout << std::endl;
		}
		else if (i % 8 == 0)
		{
			std::cout << "  ";
		}
	}
}