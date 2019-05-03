//---------------------------------------------------------------------------
#include <iostream>

#include "OtherFunc.h"

#pragma hdrstop
#include "Common.h"
#include "XFS.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

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
	TotalSectors = TotalClusters * SectorsPerCluster;
	InodeSize = Reverse16(pSuperblock->InoSize);
	InodesPerBlock = Reverse16(pSuperblock->InodesPerBlock);

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
	 cout << "\n" << "Размер блока "<< dec<< BytesPerCluster << " (0x"<< hex <<BytesPerCluster << ")" << endl;
	 cout << "Размер сектора " << dec << BytesPerSector << " (0x" << hex << BytesPerSector << ")" << endl;
	 cout << "Кол-во секторов в блоке " << dec << SectorsPerCluster << endl;
	 cout << "Общее кол-во блоков " << dec << TotalClusters << " (0x" << hex << TotalClusters << ")" << endl;
	 cout << "Общее кол-во секторов " << dec << TotalSectors << endl;
	 cout << "Размер инода " << dec << InodeSize << " (0x" << hex << InodeSize << ")" << endl;
	 cout << "Кол-во инодов в блоке " << dec << InodesPerBlock << " (0x" << hex << InodesPerBlock << ")" << endl;
	 cout << endl;
}
//---------------------------------------------------------------------------
XFS_FileSystemClass::~XFS_FileSystemClass()
{

}
//---------------------------------------------------------------------------