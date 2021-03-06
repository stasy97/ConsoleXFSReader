#include "NTFS.h"

//------------------NTFS_FileSystem---------------------------------------------

void NTFS_FileSystemClass::Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize, DWORD clusterSize)
{
	DWORD bytesRead;
	BYTE buffer[1024];
	ULARGE_INTEGER tempOffset;
	ULONG result;

	PNTFSBootRecord pBootRecord;

	ClearError();

	bytesRead = dataStorage->ReadDataByOffset(startOffset, 1024, buffer);
	if (bytesRead != 1024)
	{
		SetError();
		cout << endl << "bytesRead != 1024";
		return;
	}

	Type = FileSystemTypeEnum::NTFS;

	pBootRecord = (PNTFSBootRecord)buffer;

	Storage = dataStorage;
	StartOffset = startOffset;

	OemName = pBootRecord->OEMName;
	BytesPerSector = pBootRecord->BytesPerSector;
	SectorsPerCluster = pBootRecord->SectorsPerCluster;
	BytesPerCluster = BytesPerSector * SectorsPerCluster;
	TotalSectors = pBootRecord->TotalSectors;
	TotalClusters = TotalSectors / SectorsPerCluster;

}
//---------------------------------------------------------------------------
NTFS_FileSystemClass::NTFS_FileSystemClass(NTFS_FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// ������ Storage ��������� ���������������� ������� FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
NTFS_FileSystemClass::NTFS_FileSystemClass(FileSystemClass *srcFileSystem) : FileSystemClass(srcFileSystem)
{
	// ������ Storage ��������� ���������������� ������� FileSystemClass(srcFileSystem)
	Init(Storage, srcFileSystem->GetStartOffset(), 0, srcFileSystem->GetBytesPerSector());
}
//---------------------------------------------------------------------------
NTFS_FileSystemClass::NTFS_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize)
{
	Init(dataStorage, startOffset, diskSize, sectorSize);
}
//---------------------------------------------------------------------------
void NTFS_FileSystemClass::ShowInfo() {
	cout << "\n" << "BlockSize " << dec <<BytesPerCluster << " (0x" << std::hex << BytesPerCluster << ")" << endl;
	cout << "BytesPerSector " << dec << BytesPerSector << " (0x" << std::hex << BytesPerSector << ")" << endl;
	cout << "SectorsPerCluster " << dec << SectorsPerCluster << endl;
	cout << "TotalBlocks " << dec << TotalClusters << " (0x" << std::hex << TotalClusters << ")" << endl;
	cout << "TotalSectors " << dec << TotalSectors << endl;
}
//---------------------------------------------------------------------------
NTFS_FileSystemClass::~NTFS_FileSystemClass()
{

}