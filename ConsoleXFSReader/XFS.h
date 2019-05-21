//---------------------------------------------------------------------------
#ifndef XFSH
#define XFSH
//---------------------------------------------------------------------------
#include "Common.h"
#include "Global.h"
#include "XFS_objects.h"
//---------------------------------------------------------------------------
#pragma warn -8056
//const ULONGLONG XFS_TimeReference = 0x153B281E0FB4000; // 01.01.1904 00:00:00,00
#pragma warn .8056
//---------------------------------------------------------------------------
const WORD XFS_SBSize = 512;
#pragma pack(push, 1)
//---------------------------------------------------------------------------
//FileSystemInfo
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
//---------------------------------------------------------------------------

#pragma pack(pop)
//---------------------------------------------------------------------------
class XFS_FileSystemClass : public FileSystemClass
{
private:

	WORD InodeSize;
	WORD InodesPerBlock;
	DWORD AG_count;

protected:

	virtual void Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize = 0, WORD sectorSize = 0, DWORD clusterSize = 0);
	virtual void ShowInfo();


public:

	XFS_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize);
	XFS_FileSystemClass(XFS_FileSystemClass *srcFileSystem);
	XFS_FileSystemClass(FileSystemClass *srcFileSystem);

	LONGLONG GetOffsetByInodeId(ULONGLONG inodeId);
	//PXFS_ExtentDescriptor GetExtentDescr(ULONGLONG startOffset);

	virtual BlockIterator *GetIterator();

	virtual ~XFS_FileSystemClass();
};

//Итератор по блокам абстрактной файловой системы
//---------------------------------------------------------------------------
class BlockIterator : public Iterator<BinaryBlock>
{
protected:

	FileSystemClass* FileSystem;
	ULONGLONG StartIndex;
	ULONGLONG CurrentIndex;
	DWORD BytesPerBlock;
	ULONGLONG TotalNumberOfBlocks;
	DWORD ReadBlockCount;
	DWORD StepSizeInBlocks;
	DWORD ReadBlockSizeInBytes;

public:

	BlockIterator(
		FileSystemClass* FileSystem,
		ULONGLONG StartIndex,
		DWORD ReadBlockCount
	);
	
	ULONGLONG GetCurrentIndex() { return CurrentIndex; }
	virtual void First() { CurrentIndex = StartIndex; }
	virtual void Next() { CurrentIndex += StepSizeInBlocks; }
	virtual bool IsDone() const { return (CurrentIndex >= TotalNumberOfBlocks); }
	virtual BinaryBlock GetCurrent() const;
};
//---------------------------------------------------------------------------
//Декоратор-итератор по супер-блокам
class SB_IteratorDecorator : public Iterator<BinaryBlock>
{
protected:
	Iterator<BinaryBlock> *It;

public:
	SB_IteratorDecorator(BlockIterator *it) { It = it; }
	virtual ~SB_IteratorDecorator() { delete It; }
	virtual void First() { It->First(); cout << "XFSB!" << endl; }
	virtual void Next() ;
	virtual bool IsDone() const { return It->IsDone(); }
	virtual BinaryBlock GetCurrent() const { return It->GetCurrent(); }
};

#endif