//---------------------------------------------------------------------------
//#pragma hdrstop
//#pragma package(smart_init)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//#include "Global.h"
#include "pch.h"
#include "MyStorage.h"
#include "FileSystemExportConst.h"
#include "OtherFunc.h"
#include <iostream>
#include <string.h>
//---------------------------------------------------------------------------
StorageFragmentClass::StorageFragmentClass()
{
	FileHandle = 0;
	StartOffset = 0;
	DataSize = 0;
}
//---------------------------------------------------------------------------
StorageFragmentClass::StorageFragmentClass(wstring fileName, ULONGLONG startOffset, ULONGLONG dataSize)
{
	FileName = fileName;
	StartOffset = startOffset;
	DataSize = dataSize;

	FileHandle = 0;
}
//---------------------------------------------------------------------------
//WORD StorageFragmentClass::GetFileName(WCHAR *fileName)
//{
//	if (fileName)
//	{
//
//		//wcscpy_s(fileName, FileName.c_str());
//	}
//
//	return FileName.length();
//}
//---------------------------------------------------------------------------
ULONGLONG StorageFragmentClass::GetDataSize()
{
	return DataSize;
}
//---------------------------------------------------------------------------
ULONGLONG StorageFragmentClass::GetStartOffset()
{
	return StartOffset;
}
//---------------------------------------------------------------------------
HANDLE StorageFragmentClass::GetFileHandle()
{
	return FileHandle;
}
//---------------------------------------------------------------------------
void StorageFragmentClass::ResetFileHandle()
{
	FileHandle = 0;
}
//---------------------------------------------------------------------------
bool StorageFragmentClass::Open()
{
	FileHandle = CreateFileW(
		FileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		FileHandle = 0;
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
void StorageFragmentClass::Close()
{
	if (FileHandle)
	{
		CloseHandle(FileHandle);
		FileHandle = 0;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
StorageClass * __fastcall StorageClass::OpenStorage(const WCHAR *fileName)
{
	StorageClass *dataStorage = NULL;
	DWORD fileAttributes = GetFileAttributesW(fileName);

	if ((fileName[wcslen(fileName) - 1] == L':'))
	{
		// ��� ������� ����� ���� ������� � ������� "C:"
		dataStorage = new SimpleStorageClass(fileName);
	}
	else
	{
		// ������� ��� �����
		dataStorage = new SimpleStorageClass(fileName);
	}

	if (!dataStorage->Open())
	{
		delete dataStorage;
		dataStorage = NULL;
	}

	return dataStorage;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
StorageClass::StorageClass()
{
	Type = StorageType::ImageFile;
	DataSize = 0;
}
//---------------------------------------------------------------------------
StorageClass::~StorageClass()
{
}
//---------------------------------------------------------------------------
StorageType StorageClass::GetType()
{
	return Type;
}
//---------------------------------------------------------------------------
ULONGLONG StorageClass::GetDataSize()
{
	return DataSize;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SimpleStorageClass::SimpleStorageClass(SimpleStorageClass *srcStorage) : StorageClass()
{
	Type = srcStorage->Type;
	Fragments = srcStorage->Fragments;
	FragmentIndex = NULL;
	FragmentIndexSize = 0;
	ResetFragments();
}
//---------------------------------------------------------------------------
StorageClass *SimpleStorageClass::GetCopy()
{
	return new SimpleStorageClass(this);
}
//---------------------------------------------------------------------------
SimpleStorageClass::SimpleStorageClass(StorageType newType, vector<StorageFragmentClass> &newFragments) : StorageClass()
{
	Type = newType;
	Fragments = newFragments;
	DataSize = 0;
	FragmentIndex = NULL;
	FragmentIndexSize = 0;

	ResetFragments();
}
//---------------------------------------------------------------------------
SimpleStorageClass::SimpleStorageClass(const WCHAR *fileName) : StorageClass()
{
	wstring tempStr;
	DWORD SectorsPerCluster;
	DWORD BytesPerSector;
	DWORD TotalNumberOfClusters;
	bool result;


	DataSize = 0;

	// ���� ��������� ������ - ���������, �� ����� ���� � ���������� ������
	if (fileName[wcslen(fileName) - 1] == L':')
	{
		Type = StorageType::LogicalDrive;

		// ���������, ���� �� � ����� ������� ������� "\\.\"
		if (wcsncmp(fileName, L"\\\\.\\", 4) == 0)
		{
			// ��� ������� � ������� "\\.\X:"
			tempStr = wstring(fileName);
		}
		else
		{
			// ��� ������� ���������������� � ������� X:
			tempStr = wstring(L"\\\\.\\");
			tempStr.append(wstring(fileName));
		}
		// ���������� ������ ������� � ���������� ���

		DataSize = 4000000; //���� ��� ���������, ��� ���������� ������ �����, ��� ��� Windows �� ���������� ���
		Fragments.push_back(StorageFragmentClass(fileName, 0, DataSize));
						
	}
	else
	{
		// ������������, ��� ������� ��� �����
		Type = StorageType::ImageFile;

		const WCHAR *pBracketPosition = wcschr(fileName, '[');
		if (pBracketPosition)
			tempStr = wstring(fileName, pBracketPosition - fileName);
		else
			tempStr = wstring(fileName);

		DataSize = _GetFileSize(fileName);

		if (DataSize != 0) {

			cout << "DataSize = "<< DataSize;
			Fragments.push_back(StorageFragmentClass(fileName, 0, DataSize));
		}
	}

	FragmentIndex = NULL;
	FragmentIndexSize = 0;
	
	ResetFragments();
}
//---------------------------------------------------------------------------
SimpleStorageClass::~SimpleStorageClass()
{
	Close();
	ClearFragments();
}
//---------------------------------------------------------------------------
//WORD SimpleStorageClass::GetBaseFilePath(WCHAR *filePath)
//{
//	return Fragments[0].GetFileName(filePath);
//}
//---------------------------------------------------------------------------
void SimpleStorageClass::ResetFragments()
{
	for (vector<StorageFragmentClass>::iterator storageFragment = Fragments.begin(); storageFragment != Fragments.end(); storageFragment++)
	{
		storageFragment->ResetFileHandle();
	}
}
//---------------------------------------------------------------------------
void SimpleStorageClass::ClearFragments()
{
	// �������� ������� ���������� � ����������
	Fragments.clear();
	DataSize = 0;
}
//---------------------------------------------------------------------------
bool SimpleStorageClass::Open()
{
	if (Type == StorageType::ErrorType || Fragments.size() == 0) return false;

	// ���� �������� ������, ��� ���������� �������
	if (DataSize) Close();

	for (vector<StorageFragmentClass>::iterator storageFragment = Fragments.begin(); storageFragment != Fragments.end(); storageFragment++)
	{
		storageFragment->Open();
		DataSize += storageFragment->GetDataSize();
		std::cout <<endl<<"DataSize in fragment = "<<DataSize << "...";
	}

	return PrepareFragmentIndex();
}
//---------------------------------------------------------------------------
void SimpleStorageClass::Close()
{
	if (FragmentIndexSize) ClearFragmentIndex();

	for (vector<StorageFragmentClass>::iterator storageFragment = Fragments.begin(); storageFragment != Fragments.end(); storageFragment++)
	{
		storageFragment->Close();
	}

	DataSize = 0;
}
//---------------------------------------------------------------------------
bool SimpleStorageClass::PrepareFragmentIndex()
{
	if (!DataSize) return false;

	FragmentIndexSize = Fragments.size();
	FragmentIndex = new StorageFragmentStruct[FragmentIndexSize];

	int i = 0;

	for (vector<StorageFragmentClass>::iterator storageFragment = Fragments.begin(); storageFragment != Fragments.end(); storageFragment++, i++)
	{
		FragmentIndex[i].FileHandle = storageFragment->GetFileHandle();
		FragmentIndex[i].StartOffset = storageFragment->GetStartOffset();
		FragmentIndex[i].EndOffset = FragmentIndex[i].StartOffset + storageFragment->GetDataSize();

		/*std::cout<<""<<std::endl;
		std::cout << "FragmentIndex["<< i <<"].FileHandle = " << FragmentIndex[i].FileHandle << std::endl;
		std::cout << "FragmentIndex[" << i << "].StartOffset = " << FragmentIndex[i].StartOffset << std::endl;
		std::cout << "FragmentIndex[" << i << "].EndOffset = " << FragmentIndex[i].EndOffset << std::endl; */

	}

	return true;
}
//---------------------------------------------------------------------------
void SimpleStorageClass::ClearFragmentIndex()
{
	delete[] FragmentIndex;
	FragmentIndex = NULL;
	FragmentIndexSize = 0;
}
//---------------------------------------------------------------------------
DWORD SimpleStorageClass::ReadDataByOffset(ULONGLONG startOffset, DWORD bytesToRead, BYTE *dataBuffer, DWORD *leftToRead)
{
	LARGE_INTEGER sectorOffset;
	ULONG result;
	DWORD bytesRead;
	
	// ���������, ������ �� ������
	if (!FragmentIndexSize) return 0;

	if (FragmentIndexSize == 1) // ���������� ���������� ����� 1, ������ � ������� ������
	{
		if (!FragmentIndex[0].FileHandle) return 0;

		StorageType type = this->GetType();

		if (type == StorageType::LogicalDrive) {
			sectorOffset.QuadPart = startOffset;
		}
		else sectorOffset.QuadPart = startOffset + 0x100000;

		// ������ ������� � �����
		result = SetFilePointer(
			FragmentIndex[0].FileHandle,
			sectorOffset.LowPart,
			&sectorOffset.HighPart,
			FILE_BEGIN
		);

		if (result != sectorOffset.LowPart) return 0;

		// �������
		ReadFile(
			FragmentIndex[0].FileHandle,
			dataBuffer,
			bytesToRead,
			&bytesRead,
			NULL
		);

		if (leftToRead != NULL) *leftToRead = bytesToRead - bytesRead;
		return bytesRead;
	}
	else
	{
		if (leftToRead != NULL) *leftToRead = bytesToRead;
		return 0;
	}
}
//---------------------------------------------------------------------------
FileSystemTypeEnum RecognizeFileSystem(const BYTE *buffer, DWORD bufferSize)
{
	//// ��������� ����������� ������ �������� ������� (2048 ����)
	//if (bufferSize < 0x800) return FileSystemTypeEnum::FS_None;

	// ��������, ��� ��� ������ XFS
	WORD bootRecordSignature;
	memcpy(&bootRecordSignature, &buffer[0x400], 2);

	if (bootRecordSignature == Reverse16(0x584653))
	{
		return FileSystemTypeEnum::XFS;
	}

	return FileSystemTypeEnum::FS_None;
}
//---------------------------------------------------------------------------

