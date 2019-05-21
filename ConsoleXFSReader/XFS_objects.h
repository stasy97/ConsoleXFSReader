#pragma once
#include "Common.h"
#include "OtherFunc.h"

const WORD XFS_inodeCoreSize = 176;
const WORD XFS_MaxFileNameLength = 255;
const DWORD XFS_MaxNodeSize = 32768; //?
const DWORD XFS_MaxBTreeCacheSize = 0x400000; // ������������ ������ ����
const DWORD XFS_MaxDirSize = 0x10000; // ������������ ������ �������� (������ ������������� ������)
#pragma pack(push, 1)
enum
{
	kXFSRootParentID = 1, // ������������ ID �������� �����
	kXFSRootFolderID = 2, // ID �������� �����
	kXFSExtentsFileID = 3, // ID ����� extents overflow
	kXFSCatalogFileID = 4, // ID ����� ��������
	kXFSBadBlockFileID = 5, // ID ����� bad block
	kXFSAllocationFileID = 6, // ID ����� allocation
	kXFSStartupFileID = 7, // ID ����� startup
	kXFSAttributesFileID = 8, // ID ����� attributes
	kXFSRepairCatalogFileID = 14,
	kXFSBogusExtentFileID = 15,
	kXFSFirstUserCatalogNodeID = 16
};

enum // ��� ������ ��������
{
	kXFSFolderRecord = 0x0001, // ������ ����� - XFSCatalogFolder
	kXFSFileRecord = 0x0002, // ������ ����� - XFSCatalogFile
	kXFSFolderThreadRecord = 0x0003, // ������ ����� ������ - XFSCatalogThread
	kXFSFileThreadRecord = 0x0004  // ������ ����� ������ - XFSCatalogThread
};
//---------------------------------------------------------------------------
typedef DWORD XFS_CatalogNodeID;
//---------------------------------------------------------------------------
//typedef XFS_ExtentDescriptor XFS_ExtentRecord[8];

typedef struct XFS_ForkDataStruct
{
	ULONGLONG logicalSize;   // ������ ������
	DWORD  clumpSize;
	DWORD  totalBlocks;           // ����� ���������� ���������� ���������
	//XFS_ExtentRecord  extents;   // �������� ���������

} XFS_ForkData, *PXFS_ForkData;
//---------------------------------------------------------------------------
typedef struct XFS_CatalogKeyStruct
{
	WORD                keyLength; // ����� ����� (16 ���)
	DWORD				parentID;  // ������������� ������������ �����

} XFS_CatalogKey, *PXFS_CatalogKey;
//---------------------------------------------------------------------------
typedef struct XFS_CatalogFileStruct
{
	short int           recordType;
	WORD                flags;
	DWORD               reserved1;
	XFS_CatalogNodeID  fileID;
	DWORD               createDate;
	DWORD               contentModDate;
	DWORD               attributeModDate;
	DWORD               accessDate;
	DWORD               backupDate;
	//XFS_BSDInfo        permissions;
	//XFS_FileInfo       userInfo;
	//XFS_ExtendedFileInfo    finderInfo;
	DWORD               textEncoding;
	DWORD               reserved2;
	XFS_ForkData       dataFork;
	XFS_ForkData       resourceFork;

} XFS_CatalogFile, *PXFS_CatalogFile;

//---------------------------------------------------------------------------
enum // ��� ������ inode 
{
	kIsFile = 0x80,     /* Files */
	kIsCatalog = 0x40,     /* Folders */

};

typedef struct XFS_ExtentDescriptorStruct
{
	DWORD  startBlock;   // ����� ������� ����� ������
	DWORD  blockCount;   // ���������� ������ � ��������

} XFS_ExtentDescriptor, *PXFS_ExtentDescriptor;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

typedef struct XFS_InodeCoreStruct
{
	WORD  signature;				// "IN"									// 0x00
	WORD fileType;					// ��� ����� � ���� ������				// 0x02
	BYTE version;					// ������ inode							// 0x03
	BYTE dataForkType;				//Data fork type						// 0x04
	BYTE info[50];															// 0x05
	ULONGLONG forkDataSize;			// ������ �����							// 0x38
	ULONGLONG forkDataBlocks;		//Number of blocks in data fork			// 0x40
	DWORD hint;						//Extent size hint    zeroed?			// 0x48
	DWORD usedDataExtents;			//Number of data extents used			// 0x4A
	WORD usedAttrExtents;			//Number of extended attribute extents	// 0x50
	BYTE xattrOffset;				//Inode offset to xattr(8 byte multiples)// 0x52
	BYTE attrType;					//Extended attribute type				// 0x53

} XFS_InodeCore, *PXFS_InodeCore;

#pragma pack(pop)

//class XFS_FileObjectClass : public FileObjectClass
//{
//protected:
//
//	class XFS_FileSystemClass* FileSystem;
//	short int RecordType;
//
//	bool ForkDataDefined;
//	//XFS_ForkData *ForkData;
//
//	DWORD ParentId;
//
//	void Init(class XFS_FileSystemClass *fileSystem, LONGLONG recordId, XFS_ForkData *forkData = NULL, const WCHAR *fileName = NULL, size_t fileNameLength = 0, const WCHAR *filePath = NULL, size_t filePathLength = 0, const BYTE *recordBuffer = NULL, size_t fileRecordSize = 0);
//
//public:
//
//	XFS_FileObjectClass(XFS_FileSystemClass *fileSystem);
//	XFS_FileObjectClass(XFS_FileSystemClass *fileSystem, XFS_ForkData *forkData);
//	XFS_FileObjectClass(XFS_FileSystemClass *fileSystem, LONGLONG recordId, const WCHAR *fileName = NULL, size_t fileNameLength = 0, const WCHAR *filePath = NULL, size_t filePathLength = 0, const BYTE *recordBuffer = NULL, size_t recordSize = 0);
//
//	//virtual class FileSystemClass *GetFileSystem();
//	//class XFS_FileSystemClass *XFS_GetFileSystem();
//
//	virtual bool IsDir();
//	virtual bool IsLink();
//	virtual bool IsUsed();
//	virtual wstring PrintFileFlags();
//
//	// �������� � �������� �������
//	virtual WORD GetFileRecordType(CHAR* fileType);
//	virtual DWORD GetFileRecord(BYTE *dataBuffer = NULL);
//	virtual DWORD GetFileRecordRealSize();
//	virtual LONGLONG GetRecordOffset();
//
//	//virtual DirectoryIterator<wstring>* GetDirectoryNameIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All);
//	//virtual DirectoryIterator<FileObjectClass*>* GetDirectoryFileIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All);
//	//virtual DirectoryIterator<DirectoryEntryRecord>* GetDirectoryRecordIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All);
//
//	//XFS_ForkData *GetForkData();
//	//virtual LONGLONG GetParentId();
//
//	// �������� � ���������� ���������
//	virtual bool ReadTimeStamps();
//
//	virtual ~XFS_FileObjectClass() { };
//};