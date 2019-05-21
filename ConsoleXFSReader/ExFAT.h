#pragma once
#include "Common.h"
#include <cmath>

typedef ULONGLONG ExFAT_OEMName;

#pragma pack(push, 1)
typedef struct
{
	BYTE JMP_instr[3];
	ExFAT_OEMName OEMName;
	BYTE Padding1[61];
	ULONGLONG VolumeLength;
	BYTE FatOffset[4];
	BYTE FatLength[4];
	BYTE ClusterHeapOffset[4];
	DWORD ClustersCount;
	BYTE FirstClusterOfRootDirectory[4];
	BYTE Padding2[8];
	BYTE BytesPerSectorShift;
	BYTE SectorsPerClusterShift;
	BYTE NumberOfFats;

} ExFATBootRecord, *PExFATBootRecord;

#pragma pack(pop)

class ExFAT_FileSystemClass :
	public FileSystemClass
{
protected:
	ExFAT_OEMName OemName; //ExFAT
	virtual void Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize = 0, WORD sectorSize = 0, DWORD clusterSize = 0);
	virtual void ShowInfo();

public:

	ExFAT_FileSystemClass(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize);
	ExFAT_FileSystemClass(ExFAT_FileSystemClass *srcFileSystem);
	ExFAT_FileSystemClass(FileSystemClass *srcFileSystem);

	virtual BlockIterator *GetIterator() { return NULL; }

	virtual ~ExFAT_FileSystemClass();
};

