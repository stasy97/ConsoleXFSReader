//---------------------------------------------------------------------------
#ifndef FileSystemExportConstH
#define FileSystemExportConstH
//---------------------------------------------------------------------------
#include <wtypes.h>
//---------------------------------------------------------------------------
typedef int StorageHandle;
typedef int FileSystemHandle;
//---------------------------------------------------------------------------
const StorageHandle STORAGE_UNKNOWN = 0;
const StorageHandle STORAGE_ERROR = -1;
//---------------------------------------------------------------------------
const FileSystemHandle FILESYSTEM_UNKNOWN = 0;
const FileSystemHandle FILESYSTEM_ERROR = -1;
//---------------------------------------------------------------------------
const DWORD DefaultSectorSize = 512;
//---------------------------------------------------------------------------
enum class StorageType : int
{
	LogicalDrive,
	ImageFile,
	ErrorType
};
//---------------------------------------------------------------------------
enum class FileSystemTypeEnum : int
{
	XFS,
	NTFS,
	ExFAT,
	FS_None,
	FS_Error,
	FS_Debug
};
//---------------------------------------------------------------------------
#endif
