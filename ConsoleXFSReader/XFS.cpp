//---------------------------------------------------------------------------
#include <iostream>

#include "OtherFunc.h"

#pragma hdrstop
#include "Common.h"
#include "XFS.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//------------------FileSystem-----------------------------------------------

void XFS_FileSystemClass::Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize, DWORD clusterSize)
{
	DWORD bytesRead;
	BYTE buffer[1024];
	ULARGE_INTEGER tempOffset;
	ULONG result;

	PXFS_SuperBlock pSuperblock;

	ClearError();

	bytesRead = dataStorage->ReadDataByOffset(startOffset, 1024, buffer);
	if (bytesRead != 1024)
	{
		SetError();
		cout << endl << "bytesRead != 1024";
		return;
	}

	Type = FileSystemTypeEnum::XFS;

	pSuperblock = (PXFS_SuperBlock)buffer;

	Storage = dataStorage;
	StartOffset = startOffset;

	BytesPerCluster = Reverse32(pSuperblock->blockSize);
	BytesPerSector = Reverse16(pSuperblock->SectSize);
	SectorsPerCluster = BytesPerCluster / BytesPerSector;
	TotalClusters = Reverse64(pSuperblock->totalBlocks.QuadPart);
	TotalSectors = TotalClusters / SectorsPerCluster;
	InodeSize = Reverse16(pSuperblock->InoSize);
	InodesPerBlock = Reverse16(pSuperblock->InodesPerBlock);
	AG_count = Reverse32(pSuperblock->NumAGs);
	
}
//---------------------------------------------------------------------------
XFS_FileSystemClass::XFS_FileSystemClass(XFS_FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// Объект Storage создается инициализируется вызовом FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
XFS_FileSystemClass::XFS_FileSystemClass(FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// Объект Storage создается инициализируется вызовом FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
XFS_FileSystemClass::XFS_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize)
{
	Init(dataStorage, startOffset, diskSize, sectorSize);
}
//---------------------------------------------------------------------------
void XFS_FileSystemClass::ShowInfo() {
	 cout << "\n" << "BlockSize " << BytesPerCluster << " (0x"<< std::hex <<BytesPerCluster << ")" << endl;
	 cout << "BytesPerSector " << BytesPerSector << " (0x" << std::hex << BytesPerSector << ")" << endl;
	 cout << "SectorsPerCluster " << SectorsPerCluster << endl;
	 cout << "TotalBlocks " << TotalClusters << " (0x" << std::hex << TotalClusters << ")" << endl;
	 cout << "TotalSectors " << TotalSectors << endl;
	 cout << "InodeSize " << InodeSize << " (0x" << std::hex << InodeSize << ")" << endl;
	 cout << "InodesPerBlock " << InodesPerBlock << " (0x" << std::hex << InodesPerBlock << ")" << endl;

}
//---------------------------------------------------------------------------
LONGLONG XFS_FileSystemClass::GetOffsetByInodeId(ULONGLONG inodeId) {
	
	LONGLONG totalInodes;
	totalInodes = TotalClusters / InodesPerBlock;

	if (inodeId >= totalInodes) return -1;

	//startSector = StartOffset / BytesPerSector;

	return (StartOffset + inodeId * InodeSize);
}
//---------------------------------------------------------------------------
//PXFS_ExtentDescriptor XFS_FileSystemClass::GetExtentDescr(ULONGLONG startOffset) {
//
//}
//---------------------------------------------------------------------------
XFS_FileSystemClass::~XFS_FileSystemClass()
{
	
}
//---------------------------------------------------------------------------

//Методы итератора
//---------------------------------------------------------------------------
BlockIterator::BlockIterator(FileSystemClass* fileSystem, ULONGLONG startIndex, DWORD ReadBlock)
{
	FileSystem = fileSystem;
	StartIndex = startIndex;
	CurrentIndex = startIndex;
	BytesPerBlock = FileSystem->GetBytesPerCluster();
	TotalNumberOfBlocks = FileSystem->GetTotalClusters();
	ReadBlockCount = ReadBlock;
	ReadBlockSizeInBytes = BytesPerBlock * ReadBlockCount;
	StepSizeInBlocks = 1;
}

BinaryBlock BlockIterator::GetCurrent() const
{
	BinaryBlock outBlock;
	outBlock.resize(ReadBlockSizeInBytes);
	DWORD readSize = FileSystem->ReadClustersByNumber(CurrentIndex, StepSizeInBlocks, &outBlock.front());

	outBlock.resize(readSize);

	return outBlock;
}
//Проверка, что блок - это суперблок по сигнатуре "XFSB"
void SB_IteratorDecorator::Next()
{
	BinaryBlock currentBlock;
	currentBlock.resize(4096);

	while (!It->IsDone())
	{
		It->Next();
		currentBlock = It->GetCurrent();
		currentBlock.resize(XFS_SBSize);
		
		if (currentBlock[0] == 0x58 && currentBlock[1] == 0x46 && currentBlock[2] == 0x53 && currentBlock[3] == 0x42) //XFSB
		{
			cout << "XFSB!" << endl;
			break;
		}
		
	}
}

BlockIterator * XFS_FileSystemClass::GetIterator()
{ 
	return new BlockIterator(this, 0, 1); 
}