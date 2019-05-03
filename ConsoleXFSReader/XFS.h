//---------------------------------------------------------------------------
#ifndef XFSH
#define XFSH
//---------------------------------------------------------------------------
#include "Common.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const WORD XFS_MaxFileNameLength = 255;
const DWORD XFSP_MaxNodeSize = 32768;
const DWORD XFS_MaxBTreeCacheSize = 0x400000; // Максимальный размер кэша
const DWORD XFS_MaxDirSize = 0x10000; // Максимальный размер каталога (размер резервируемой памяти)
//---------------------------------------------------------------------------
#pragma warn -8056
const ULONGLONG XFS_TimeReference = 0x153B281E0FB4000; // 01.01.1904 00:00:00,00
#pragma warn .8056
//---------------------------------------------------------------------------
#pragma pack(push, 1)

typedef struct XFS_ExtentDescriptorStruct
{
	DWORD  startBlock;   // Номер первого кластера
	DWORD  blockCount;   // Количество кластеров в экстенте

} XFS_ExtentDescriptor, *PXFS_ExtentDescriptor;
//---------------------------------------------------------------------------
typedef XFS_ExtentDescriptor XFS_ExtentRecord[8];
//---------------------------------------------------------------------------
typedef struct XFS_ForkDataStruct
{
	ULONGLONG logicalSize;   // Размер данных
	DWORD  clumpSize;
	DWORD  totalBlocks;           // Общее количество выделенных кластеров
	XFS_ExtentRecord  extents;   // Описание экстентов

} XFS_ForkData, *PXFS_ForkData;

typedef DWORD XFS_CatalogNodeID;
//---------------------------------------------------------------------------
typedef struct XFS_SuperBlockStruct
{
	DWORD  signature;				// "XFSB"						// 0x00
	DWORD blockSize;				// Размер блока в байтах		// 0x04
	ULARGE_INTEGER totalBlocks;     // Общее количество блоков      // 0x08

	DWORD Info[8];           		// Num blocks in real-time		// 0x10
									// Num extents in real-time		// 0x18
									//UUID							// 0x20
	
	ULARGE_INTEGER FBJournal;		//First block of journal		// 0x30
	ULARGE_INTEGER RootInode;		//Root directory's inode		// 0x38
	

	DWORD RTInfo[5];			//Real-time extents bitmap inode	// 0x40
								//Real-time bitmap summary inode	// 0x48
								//Real-time extent size (in blocks)	// 0x50

	DWORD AGSize;				//AG size(in blocks)				//0x54
	DWORD NumAGs;				//Number of AGs						//0ч58
	DWORD RTNum;				//Num of real - time bitmap blocks	//0x5С
	DWORD JNum;					//Num of journal blocks				//0x60
	
	WORD version;				//File system version and flags		//0x64
	WORD SectSize;				//Sector size						//0x66
	WORD InoSize;				//Inode size						//0x68
	WORD InodesPerBlock;		//Inodes/block						//0x6A


	//XFS_ForkData   allocationFile;                          // 0x70

} XFS_SuperBlock, *PXFS_SuperBlock;

#pragma pack(pop)
//---------------------------------------------------------------------------
class XFS_FileSystemClass : public FileSystemClass
{
private:

	WORD InodeSize;
	WORD InodesPerBlock;

protected:

	virtual void Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize = 0, WORD sectorSize = 0, DWORD clusterSize = 0);
	virtual void ShowInfo();


public:

	XFS_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize);
	XFS_FileSystemClass(XFS_FileSystemClass *srcFileSystem);
	XFS_FileSystemClass(FileSystemClass *srcFileSystem);

	virtual ~XFS_FileSystemClass();
};
//---------------------------------------------------------------------------
#endif