//---------------------------------------------------------------------------
#ifndef LibFileSystemH
#define LibFileSystemH
//---------------------------------------------------------------------------
FileSystemClass* CreateFileSystem(FileSystemTypeEnum fsType, StorageClass *dataStorage, ULONGLONG startOffset = 0, ULONGLONG diskSize = 0, WORD sectorSize = DefaultSectorSize);
//---------------------------------------------------------------------------
#endif
