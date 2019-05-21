#pragma once
#include "Common.h"

typedef ULONGLONG NTFS_OEMName;

#pragma pack(push, 1)
typedef struct
{
	BYTE JMP_instr[3];
	NTFS_OEMName OEMName;
	WORD BytesPerSector;
	BYTE SectorsPerCluster;
	BYTE Padding[23];
	ULONGLONG TotalSectors;

} NTFSBootRecord, *PNTFSBootRecord;

#pragma pack(pop)

class NTFS_FileSystemClass :
	public FileSystemClass
{
protected:
	NTFS_OEMName OemName; //NTFS
	virtual void Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize = 0, WORD sectorSize = 0, DWORD clusterSize = 0);
	virtual void ShowInfo();

public:

	NTFS_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize);
	NTFS_FileSystemClass(NTFS_FileSystemClass *srcFileSystem);
	NTFS_FileSystemClass(FileSystemClass *srcFileSystem);

	virtual BlockIterator *GetIterator() { return NULL; }

	virtual ~NTFS_FileSystemClass();
};


