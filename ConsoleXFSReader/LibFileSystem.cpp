//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MyStorage.h"
#include "Common.h"
#include "XFS.h"
#include "NTFS.h"
#include "ExFAT.h"
#include "FileSystemExportConst.h"
#include "LibFileSystem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
//Фабричный метод
//---------------------------------------------------------------------------
FileSystemClass* CreateFileSystem(FileSystemTypeEnum fsType, StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize, WORD sectorSize)
{
	switch (fsType)
	{
	case FileSystemTypeEnum::XFS: return new XFS_FileSystemClass(dataStorage, startOffset, diskSize, sectorSize);
	case FileSystemTypeEnum::NTFS: return new NTFS_FileSystemClass(dataStorage, startOffset, diskSize, sectorSize);
	case FileSystemTypeEnum::ExFAT: return new ExFAT_FileSystemClass(dataStorage, startOffset, diskSize, sectorSize);
	default: return NULL;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

