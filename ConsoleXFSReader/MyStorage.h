//---------------------------------------------------------------------------
#ifndef StorageH
#define StorageH
//---------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <list>
#include <wtypes.h>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
#include "FileSystemExportConst.h"
//---------------------------------------------------------------------------
//#pragma warn -8056
const ULONGLONG ExtraPartitionFlag = 0xFFFFFFFFFFFFFFFF;
//#pragma warn .8056
//---------------------------------------------------------------------------
#pragma pack(push, 1)
//---------------------------------------------------------------------------
typedef struct
{
	BYTE BootFlag;
	BYTE StartC;
	BYTE StartH;
	BYTE StartS;
	BYTE Type;
	BYTE LastC;
	BYTE LastH;
	BYTE LastS;
	ULONG StartLBA;
	ULONG TotalSectors;

} DOS_PartitionRecord;
//---------------------------------------------------------------------------
typedef struct
{
	BYTE Signature[8];
	DWORD Revision;
	DWORD HeaderSize;
	DWORD HeaderCRC;
	DWORD Reserved1;
	ULONGLONG CurrentLBA;
	ULONGLONG BackupLBA;
	ULONGLONG FirstUsableLBA;
	ULONGLONG LastUsableLBA;
	BYTE DiskGUID[16];
	ULONGLONG PartitionEntriesStartLBA;
	DWORD NumberOfPartitionEntries;
	DWORD SizeOfPartitionEntry;
	DWORD PartitionArrayCRC;
	BYTE Reserved2[420];

} PrimaryGPTHeader, *PPrimaryGPTHeader;
//---------------------------------------------------------------------------
typedef struct
{
	BYTE TypeGuid[16];
	BYTE UniqueGuid[16];
	ULONGLONG FirstLBA;
	ULONGLONG LastLBA;
	ULONGLONG Flags;
	WCHAR Name[36];

} GPT_PartitionEntry, *PGPT_PartitionEntry;
//---------------------------------------------------------------------------
#pragma pack(pop)
//---------------------------------------------------------------------------
enum class ImageNamingType : int
{
	SingleFile,
	VariableExtension1, // FileName.001, 002, 003 ...
	VariableExtension2, // FileName.D001, D002, D003 ...
	VariableName1,      // FileName-f001.ext, FileName-f002.ext, FileName-f003.ext ... (VMware)
	VariableName2,      // imgOFFSET1.bin, imgOFFSET2.bin ... (PC-3000)
	VariableName3       // FileName01, FileName02, FileName03 ...

};
//---------------------------------------------------------------------------
typedef struct
{
	HANDLE FileHandle;
	ULONGLONG StartOffset;
	ULONGLONG EndOffset;

} StorageFragmentStruct;
//---------------------------------------------------------------------------
class StorageFragmentClass
{
private:
	HANDLE FileHandle;
	wstring FileName;
	ULONGLONG StartOffset;
	ULONGLONG DataSize;

public:
	StorageFragmentClass();
	StorageFragmentClass(wstring fileName, ULONGLONG startOffset, ULONGLONG dataSize);

	//WORD GetFileName(WCHAR *fileName = NULL);
	ULONGLONG GetDataSize();
	ULONGLONG GetStartOffset();
	HANDLE GetFileHandle();
	void ResetFileHandle();

	bool Open();
	void Close();
};
//---------------------------------------------------------------------------
// Абстрактрый машинный носитель
//---------------------------------------------------------------------------
class StorageClass
{
protected:

	StorageType Type;
	ULONGLONG DataSize; // Позволяет узнать общий размер данных носителя и служит индикатором открытия (у закрытого носителя равен 0)

public:

	// Общедоступные фабричные методы
	static StorageClass * __fastcall OpenStorage(const WCHAR *fileName);

	StorageClass();
	virtual ~StorageClass();

	StorageType GetType();
	ULONGLONG GetDataSize();
	//virtual WORD GetBaseFilePath(WCHAR *filePath = NULL) = 0;

	virtual bool Open() = 0;
	virtual void Close() = 0;

	// Копирующий конструктор
	virtual StorageClass *GetCopy() = 0;

	// Самый Главный Метод (чтение блока данных с носителя)
	virtual DWORD ReadDataByOffset(ULONGLONG startOffset, DWORD bytesToRead, BYTE *dataBuffer, DWORD *leftToRead = NULL) = 0;
};
//---------------------------------------------------------------------------
// Машинный носитель типа "логический диск" или "файл-образ" (в том числе составной)
//---------------------------------------------------------------------------
class SimpleStorageClass : public StorageClass
{
protected:
	vector<StorageFragmentClass> Fragments;
	StorageFragmentStruct *FragmentIndex;
	int FragmentIndexSize;

	void ResetFragments();
	void ClearFragments();
	bool PrepareFragmentIndex();
	void ClearFragmentIndex();

public:
	//StorageClass();
	SimpleStorageClass(StorageType newType, vector<StorageFragmentClass> &newFragments);
	SimpleStorageClass(SimpleStorageClass *srcStorage);
	SimpleStorageClass(const WCHAR *fileName);
	virtual ~SimpleStorageClass();

//	virtual WORD GetBaseFilePath(WCHAR *filePath = NULL);

	virtual bool Open();
	virtual void Close();

	// Копирующий конструктор
	virtual StorageClass *GetCopy();

	// Самый Главный Метод (чтение блока данных с носителя)
	virtual DWORD ReadDataByOffset(ULONGLONG startOffset, DWORD bytesToRead, BYTE *dataBuffer, DWORD *leftToRead = NULL);
};
//---------------------------------------------------------------------------
FileSystemTypeEnum RecognizeFileSystem(const BYTE *buffer, DWORD bufferSize);
//---------------------------------------------------------------------------
#endif
