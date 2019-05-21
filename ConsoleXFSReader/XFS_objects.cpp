#include "XFS_objects.h"

////------------------XFS_FileObject-----------------------------------------------
//void XFS_FileObjectClass::Init(class XFS_FileSystemClass *fileSystem, LONGLONG recordId, XFS_ForkData *forkData, const WCHAR *fileName, size_t fileNameLength, const WCHAR *filePath, size_t filePathLength, const BYTE *recordBuffer, size_t fileRecordSize)
//{
//	// Исходная инициализация
//	FileSystem = fileSystem;
//
//	FileRecordSize = 0;
//	RecordType = 0;
//
//	ParentId = 0;
//
//	if (recordId >= 0)
//	{
//		RecordIdDefined = true;
//		RecordId = recordId;
//		if (RecordId == kXFSRootFolderID)
//		{
//			RootDir = true;
//			RecordType = kXFSFolderRecord;
//		}
//		else RootDir = false;
//	}
//
//	if (fileName != NULL)
//	{
//		FileNameLength = (fileNameLength > 0 ? fileNameLength : wcslen(fileName));
//
//		if (FileNameLength > 0)
//		{
//			FileName = new WCHAR[FileNameLength + 1];
//			wcsncpy_s(FileName, fileNameLength, fileName, FileNameLength);
//			FileName[FileNameLength] = 0;
//			FileNameDefined = true;
//		}
//	}
//
//	if (filePath != NULL)
//	{
//		FilePathLength = (filePathLength > 0 ? filePathLength : wcslen(filePath));
//
//		if (FilePathLength > 0)
//		{
//			FilePath = new WCHAR[FilePathLength + 1];
//			wcsncpy_s(FilePath, fileNameLength, filePath, FilePathLength);
//			FilePath[FilePathLength] = 0;
//			FilePathDefined = true;
//		}
//	}
//
//	if (recordBuffer != NULL)
//	{
//		PXFS_CatalogKey catalogKey = (PXFS_CatalogKey)recordBuffer;
//		WORD keyLength = Reverse16(catalogKey->keyLength);
//		ParentId = Reverse32(catalogKey->parentID);
//
//		RecordBuffer = new BYTE[fileRecordSize - keyLength - sizeof(WORD)];
//		memcpy(RecordBuffer, &recordBuffer[keyLength + sizeof(WORD)], fileRecordSize - keyLength - sizeof(WORD));
//		RecordBufferDefined = true;
//		FileRecordSize = fileRecordSize - keyLength - sizeof(WORD);
//
//		RecordType = Reverse16(*((WORD*)RecordBuffer));
//
//		//if (RecordType == kXFSFileRecord)
//		//{
//		//	RecordId = Reverse32(((PXFS_CatalogFile)RecordBuffer)->fileID);
//		//	RecordIdDefined = true;
//
//		//	ForkData = &(((XFS_CatalogFileStruct*)RecordBuffer)->dataFork);
//		//	ForkDataDefined = true;
//
//		//	// Считать информацию о потоке данных
//		//	AddDataStream(new XFS_DataStreamClass(fileSystem, RecordId, ForkData));
//		//	DataStreamsDefined = true;
//		//}
//		//else if (RecordType == kXFSFolderRecord)
//		//{
//		//	RecordId = Reverse32(((PXFS_CatalogFolder)RecordBuffer)->folderID);
//		//	RecordIdDefined = true;
//		//}
//	}
//	//else if (forkData != NULL)
//	//{
//	//	ForkDataDefined = true;
//	//	ForkData = forkData;
//
//	//	// Считать информацию о потоке данных
//	//	AddDataStream(new XFS_DataStreamClass(fileSystem, RecordId, forkData));
//	//	DataStreamsDefined = true;
//	//}
//	//else if (recordId >= 0)
//	//{
//	//	// Файловой записи нет, однако известен идентификатор файла
//	//	if (GetFileRecord() != 0)
//	//	{
//	//		// Файловая запись считана
//	//		if (RecordType == kXFSFileRecord)
//	//		{
//	//			ForkData = &(((XFS_CatalogFileStruct*)RecordBuffer)->dataFork);
//	//			ForkDataDefined = true;
//
//	//			// Считать информацию о потоке данных
//	//			AddDataStream(new XFS_DataStreamClass(fileSystem, RecordId, ForkData));
//	//			DataStreamsDefined = true;
//	//		}
//	//	}
//	//}
//}
////---------------------------------------------------------------------------
//XFS_FileObjectClass::XFS_FileObjectClass(class XFS_FileSystemClass *fileSystem)
//{
//	// По умолчанию создается файл корневого каталога файловой системы
//	Init(fileSystem, kXFSRootFolderID, NULL, L".", 1, ROOT_DIR, ROOT_DIR_LENGTH);
//}
////---------------------------------------------------------------------------
//XFS_FileObjectClass::XFS_FileObjectClass(class XFS_FileSystemClass *fileSystem, XFS_ForkData *forkData)
//{
//	Init(fileSystem, -1, forkData);
//}
////---------------------------------------------------------------------------
//XFS_FileObjectClass::XFS_FileObjectClass(
//	class XFS_FileSystemClass *fileSystem,
//	LONGLONG recordId,
//	const WCHAR *fileName,
//	size_t fileNameLength,
//	const WCHAR *filePath,
//	size_t filePathLength,
//	const BYTE *recordBuffer,
//	size_t recordSize
//)
//{
//	if (recordId == kXFSRootFolderID)
//	{
//		Init(fileSystem, kXFSRootFolderID, NULL, L".", 1, ROOT_DIR, ROOT_DIR_LENGTH);
//	}
//	else
//	{
//		Init(fileSystem, recordId, NULL, fileName, fileNameLength, filePath, filePathLength, recordBuffer, recordSize);
//	}
//}
//
////---------------------------------------------------------------------------
////class FileSystemClass *XFS_FileObjectClass::GetFileSystem()
////{
////	return XFS_GetFileSystem();
////}
////---------------------------------------------------------------------------
////class XFS_FileSystemClass *XFS_FileObjectClass::XFS_GetFileSystem()
////{
////	return FileSystem;
////}
////---------------------------------------------------------------------------
////DWORD XFS_FileObjectClass::GetFileRecord(BYTE *dataBuffer)
////{
////	WCHAR *fullPath;
////	WORD fullPathLength;
////	DWORD readSize;
////
////	if (!RecordBufferDefined)
////	{
////		// Если известен путь к файлу
////		if (RecordIdDefined)
////		{
////			RecordBuffer = new BYTE[FileSystem->GetFileRecordSize()];
////
////			WCHAR fileName[XFS_MaxFileNameLength + 1] = L"";
////			//readSize = FileSystem->XFS_ReadFileRecordByRecordId(RecordId, RecordBuffer, fileName, &ParentId);
////
////
////			if (readSize == 0)
////			{
////				delete[] RecordBuffer;
////				return 0;
////			}
////
////			if (!FileNameDefined)
////			{
////				FileNameLength = wcslen(fileName);
////				if (FileNameLength > 0)
////				{
////					FileName = new WCHAR[FileNameLength + 1];
////					wcscpy(FileName, fileName);
////					FileNameDefined = true;
////				}
////			}
////
////			RecordType = Reverse16(*((WORD*)RecordBuffer));
////		}
////		else
////		{
////			return 0;
////		}
////
////		FileRecordSize = readSize;
////		RecordBufferDefined = true;
////	}
////
////	if (dataBuffer != NULL) memcpy(dataBuffer, RecordBuffer, FileRecordSize);
////	return FileRecordSize;
////}
////---------------------------------------------------------------------------
//DWORD XFS_FileObjectClass::GetFileRecordRealSize()
//{
//	if (!RecordBufferDefined) return 0;
//
//	return FileRecordSize; // С запасом при размере имени файла 256 символов
//}
////---------------------------------------------------------------------------
////LONGLONG XFS_FileObjectClass::GetParentId()
////{
////	if (!RecordIdDefined) return -1;
////	if (!RecordBufferDefined)
////	{
////		if (GetFileRecord() == 0) return -1;
////	}
////
////	return (LONGLONG)ParentId;
////}
////---------------------------------------------------------------------------
////bool XFS_FileObjectClass::IsDir()
////{
////	if (!RecordIdDefined) return false;
////	if (!RecordBufferDefined)
////	{
////		if (GetFileRecord() == 0) return false;
////	}
////
////	return XFS_IsDirectory(RecordBuffer);
////}
////---------------------------------------------------------------------------
//bool XFS_FileObjectClass::IsLink()
//{
//	if (!RecordIdDefined) return false;
//	if (!RecordBufferDefined)
//	{
//		if (GetFileRecord() == 0) return false;
//	}
//
//	if (RecordType == kXFSFolderThreadRecord) return true;
//	else if (RecordType == kXFSFileThreadRecord) return true;
//
//	return false;
//}
////---------------------------------------------------------------------------
//bool XFS_FileObjectClass::IsUsed()
//{
//	return true;
//}
////---------------------------------------------------------------------------
//WORD XFS_FileObjectClass::GetFileRecordType(CHAR* fileType)
//{
//	short int recordType;
//
//	if (!RecordIdDefined) return false;
//	if (!RecordBufferDefined)
//	{
//		if (GetFileRecord() == 0) return 0;
//	}
//
//	switch (RecordType)
//	{
//	case kXFSFileRecord:
//	{
//		strcpy_s(fileType, strlen("Файл"),"Файл");
//		return strlen(fileType);
//	}
//	default: return 0;
//	}
//
//	// Ошибка
//	return 0;
//}
//
