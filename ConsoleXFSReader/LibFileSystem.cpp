//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MyStorage.h"
#include "Common.h"
#include "XFS.h"
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
	default: return NULL;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

