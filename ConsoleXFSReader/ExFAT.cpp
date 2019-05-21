#include "ExFAT.h"

//------------------ExFAT_FileSystem---------------------------------------------

void ExFAT_FileSystemClass::Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize, DWORD clusterSize)
{
	DWORD bytesRead;
	BYTE buffer[1024];
	ULARGE_INTEGER tempOffset;
	ULONG result;

	PExFATBootRecord pBootRecord;

	ClearError();

	bytesRead = dataStorage->ReadDataByOffset(startOffset, 1024, buffer);
	if (bytesRead != 1024)
	{
		SetError();
		cout << endl << "bytesRead != 1024";
		return;
	}

	Type = FileSystemTypeEnum::ExFAT;

	pBootRecord = (PExFATBootRecord)buffer;

	Storage = dataStorage;
	StartOffset = startOffset;

	OemName = pBootRecord->OEMName;
	TotalSectors = pBootRecord->VolumeLength;
	BytesPerSector = std::pow(2, pBootRecord->BytesPerSectorShift);
	SectorsPerCluster = std::pow(2, pBootRecord->SectorsPerClusterShift);
	BytesPerCluster = BytesPerSector * SectorsPerCluster;
	TotalClusters = pBootRecord->ClustersCount;
	
}
//---------------------------------------------------------------------------
ExFAT_FileSystemClass::ExFAT_FileSystemClass(ExFAT_FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// Объект Storage создается инициализируется вызовом FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
ExFAT_FileSystemClass::ExFAT_FileSystemClass(FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// Объект Storage создается инициализируется вызовом FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
ExFAT_FileSystemClass::ExFAT_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize)
{
	Init(dataStorage, startOffset, diskSize, sectorSize);
}
//---------------------------------------------------------------------------
void ExFAT_FileSystemClass::ShowInfo() {
	cout << "\n" << "BlockSize " << dec << BytesPerCluster << endl;
	cout << "BytesPerSector " << dec << BytesPerSector << endl;
	cout << "SectorsPerCluster " << dec << SectorsPerCluster << endl;
	cout << "TotalBlocks " << dec << TotalClusters << endl;
	cout << "TotalSectors " << dec << TotalSectors << endl;
}
//---------------------------------------------------------------------------
ExFAT_FileSystemClass::~ExFAT_FileSystemClass()
{
}

