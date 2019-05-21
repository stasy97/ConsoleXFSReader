//---------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <stack>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "Common.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
FileSystemClass::FileSystemClass()
{
	SetError();

	memset(Serial, 0, sizeof(Serial));
	SerialLength = 0;

	BytesPerSector = 0;
	SectorsPerCluster = 0;
	BytesPerCluster = 0;
	TotalSectors = 0;
	TotalClusters = 0;
	StartOffset = 0;

	ClusterBitmapCacheDefined = false;
	ClusterBitmapCache = NULL;
	ClusterBitmapSize = NULL;
	ClusterBitmapCacheFirstCluster = 0;
	ClusterBitmapCacheLastCluster = 0;

	Storage = NULL;
	StorageAllocated = false;
}
//---------------------------------------------------------------------------
FileSystemClass::FileSystemClass(FileSystemClass *srcFileSystem)
{
	SetError();

	memset(Serial, 0, sizeof(Serial));
	memcpy(Serial, srcFileSystem->Serial, srcFileSystem->SerialLength);
	SerialLength = srcFileSystem->SerialLength;

	BytesPerSector = srcFileSystem->BytesPerSector;
	SectorsPerCluster = srcFileSystem->SectorsPerCluster;
	BytesPerCluster = srcFileSystem->BytesPerCluster;
	TotalSectors = srcFileSystem->TotalSectors;
	TotalClusters = srcFileSystem->TotalClusters;
	StartOffset = srcFileSystem->StartOffset;

	ClusterBitmapCacheDefined = false;
	ClusterBitmapCache = NULL;
	ClusterBitmapSize = NULL;
	ClusterBitmapCacheFirstCluster = 0;
	ClusterBitmapCacheLastCluster = 0;

	Storage = srcFileSystem->Storage->GetCopy();
	Storage->Open();
	StorageAllocated = true;
}
//---------------------------------------------------------------------------
FileSystemClass::~FileSystemClass()
{
	if (ClusterBitmapCacheDefined)	delete[] ClusterBitmapCache;

	if (StorageAllocated)
	{
		Storage->Close();
		delete Storage;
		Storage = NULL;
		StorageAllocated = false;
	}
}
//---------------------------------------------------------------------------
FileSystemTypeEnum FileSystemClass::GetType()
{
	return Type;
}
//---------------------------------------------------------------------------
wstring GetFileSystemTypeStringW(FileSystemTypeEnum fsType)
{
	switch (fsType)
	{
	case FileSystemTypeEnum::XFS:  return L"XFS";
	default:    return L"�� ����������";
	}
}
//---------------------------------------------------------------------------
//string FileSystemClass::GetTypeString()
//{
//	return ws2s(GetFileSystemTypeStringW(Type));
//}
//---------------------------------------------------------------------------
wstring FileSystemClass::GetTypeStringW()
{
	return GetFileSystemTypeStringW(Type);
}
//---------------------------------------------------------------------------
const BYTE * FileSystemClass::GetSerialPtr()
{
	return Serial;
}
//---------------------------------------------------------------------------
string FileSystemClass::GetSerialString()
{
	stringstream outStr;
	for (WORD i = 0; i < SerialLength; i++)
	{
		outStr << hex << uppercase << setfill('0') << setw(2) << int(Serial[i]);
	}

	return outStr.str();
}
//---------------------------------------------------------------------------
//wstring FileSystemClass::GetSerialStringW()
//{
//	return s2ws(GetSerialString());
//}
//---------------------------------------------------------------------------
ULONGLONG FileSystemClass::GetStartOffset()
{
	return StartOffset;
}
//---------------------------------------------------------------------------
WORD FileSystemClass::GetBytesPerSector() const
{
	return BytesPerSector;
}
//---------------------------------------------------------------------------
WORD FileSystemClass::GetSectorsPerCluster()
{
	return SectorsPerCluster;
}
//---------------------------------------------------------------------------
DWORD FileSystemClass::GetBytesPerCluster()
{
	return BytesPerCluster;
}
//---------------------------------------------------------------------------
ULONGLONG FileSystemClass::GetTotalClusters()
{
	return TotalClusters;
}
//---------------------------------------------------------------------------
ULONGLONG FileSystemClass::GetTotalSectors()
{
	return TotalSectors;
}
//---------------------------------------------------------------------------
LONGLONG FileSystemClass::GetSectorIdByOffset(ULONGLONG dataOffset) const
{
	if (dataOffset < StartOffset) return -1;
	if (dataOffset > StartOffset + TotalSectors * BytesPerSector) return -1;

	return dataOffset / BytesPerSector;
}
//---------------------------------------------------------------------------
LONGLONG FileSystemClass::GetOffsetBySectorId(ULONGLONG sectorId) const
{
	LONGLONG startSector;

	startSector = StartOffset / BytesPerSector;

	if (sectorId < startSector) return -1;
	if (sectorId - startSector >= TotalSectors) return -1; // ����� ������� ������� � ����

	return sectorId * BytesPerSector;
}
//---------------------------------------------------------------------------
LONGLONG FileSystemClass::GetClusterIdBySector(ULONGLONG sectorId) const
{
	LONGLONG startSector;
	ULONGLONG sectorOffset;

	sectorOffset = sectorId * BytesPerSector;
	startSector = StartOffset / BytesPerSector;

	if (sectorOffset < StartOffset) return -1;
	if (sectorId - startSector >= TotalSectors) return -1; // ����� ������� ������� � ����

	return ((sectorOffset - StartOffset) / BytesPerCluster);
}
//---------------------------------------------------------------------------
LONGLONG FileSystemClass::GetSectorIdByCluster(ULONGLONG clusterId) const
{
	LONGLONG startSector;

	if (clusterId >= TotalClusters) return -1;

	startSector = StartOffset / BytesPerSector;

	return (startSector + clusterId * SectorsPerCluster);
}
//---------------------------------------------------------------------------
LONGLONG FileSystemClass::GetOffsetByClusterId(ULONGLONG clusterId) const
{
	if (clusterId >= TotalClusters) return -1;

	return GetOffsetBySectorId(GetSectorIdByCluster(clusterId));
}
//---------------------------------------------------------------------------
DWORD FileSystemClass::ReadSectorsByNumber(ULONGLONG sectorId, DWORD numberOfSectors, BYTE *dataBuffer, ULONGLONG *startOffset)
{
	DWORD totalBytesToRead;
	DWORD totalBytesRead;
	DWORD bytesToRead;
	DWORD leftToRead;
	DWORD bytesRead;
	LONGLONG startSector;
	LARGE_INTEGER sectorOffset;
	ULONG result;

	// ������ �������
	sectorOffset.QuadPart = sectorId * BytesPerSector;

	if (startOffset != NULL)
	{
		*startOffset = sectorOffset.QuadPart;
	}

	startSector = StartOffset / BytesPerSector;

	if (sectorOffset.QuadPart < StartOffset) return 0;
	if (sectorId - startSector >= TotalSectors) return 0; // ����� ������� ������� � ����

	totalBytesToRead = BytesPerSector * numberOfSectors;
	bytesToRead = totalBytesToRead;
	totalBytesRead = 0;

	while (true)
	{
		//leftToRead = 0xFFFFFFFF;
		bytesRead = Storage->ReadDataByOffset(sectorOffset.QuadPart, bytesToRead, &dataBuffer[totalBytesRead], &leftToRead);

		if (bytesRead == 0)
		{
			return 0; // ������
		}

		totalBytesRead += bytesRead;

		if (totalBytesRead == totalBytesToRead) break; // ������ ��������� �������

		// ���������� ����� ������ ���������� �����, ����������� ����������
		sectorOffset.QuadPart += bytesRead;
		bytesToRead = leftToRead;
	}

	return totalBytesRead;
}
//---------------------------------------------------------------------------
DWORD FileSystemClass::ReadDataByOffset(ULONGLONG startOffset, DWORD bytesToRead, BYTE *outDataBuffer, DWORD *outLeftToRead)
{
	ULONGLONG leftToRead;
	ULONGLONG realDataStreamSize = TotalClusters * BytesPerCluster;
	DWORD startReadCluster = startOffset / BytesPerCluster;
	DWORD bufferOffset = startOffset % BytesPerCluster;

	DWORD clustersToRead = (bufferOffset + bytesToRead + BytesPerCluster - 1) / BytesPerCluster;
	clustersToRead = (startReadCluster + clustersToRead > TotalClusters ? TotalClusters - startReadCluster : clustersToRead);

	// ������� ������������� �����, ������ ��� � ���������� ������ ����� ���� ��������� ������, ��� ���������

	DWORD dataBufferSize = clustersToRead * BytesPerCluster;
	BYTE *dataBuffer = new BYTE[dataBufferSize];
	memset(dataBuffer, 0, dataBufferSize);

	DWORD totalReadSize = 0;

	do
	{
		DWORD readSize = ReadClustersByNumber(
			startReadCluster + totalReadSize / BytesPerCluster,
			clustersToRead - totalReadSize / BytesPerCluster,
			&dataBuffer[totalReadSize],
			&leftToRead
		);

		if (readSize == 0)
		{
			delete[] dataBuffer;
			if (outLeftToRead != NULL) *outLeftToRead = bytesToRead;
			return 0;
		}

		totalReadSize += readSize;

	} while (totalReadSize < clustersToRead*BytesPerCluster && startOffset + totalReadSize < realDataStreamSize);

	// ��� ��� ������������� ���������� ������ ������
	totalReadSize -= bufferOffset;
	totalReadSize = (totalReadSize > bytesToRead ? bytesToRead : totalReadSize);

	memcpy(outDataBuffer, &dataBuffer[bufferOffset], totalReadSize);

	delete[] dataBuffer;
	if (outLeftToRead != NULL) *outLeftToRead = 0;
	return totalReadSize;
}
//---------------------------------------------------------------------------
DWORD FileSystemClass::ReadClustersByNumber(ULONGLONG clusterId, DWORD numberOfClusters, BYTE *dataBuffer, ULONGLONG *startOffset) const
{
	if (clusterId >= TotalClusters) return 0; // ����� ������� �������� � ����

	// ������ �������
	LARGE_INTEGER clusterOffset;
	
	if (startOffset != NULL) clusterOffset.QuadPart = *startOffset + StartOffset + clusterId * BytesPerCluster;
	else clusterOffset.QuadPart = StartOffset + clusterId * BytesPerCluster;

	if (startOffset != NULL)
	{
		*startOffset = clusterOffset.QuadPart;
	}

	DWORD totalBytesToRead = BytesPerCluster * numberOfClusters;
	DWORD bytesToRead = totalBytesToRead;
	DWORD totalBytesRead = 0;

 	while (true)
	{
		DWORD leftToRead; //leftToRead = 0xFFFFFFFF;
		DWORD bytesRead =
			Storage->ReadDataByOffset(clusterOffset.QuadPart, bytesToRead, &dataBuffer[totalBytesRead], &leftToRead);

		if (bytesRead == 0)
		{
			return totalBytesRead; // ������
		}

		totalBytesRead += bytesRead;

		if (totalBytesRead >= totalBytesToRead) break; // ������ ��������� �������

	// ���������� ����� ������ ���������� �����, ����������� ���������� (��������� ��� �������, ����������� �� �����)
		clusterOffset.QuadPart += bytesRead;
		bytesToRead = leftToRead;
	}

	return totalBytesRead;
}
//---------------------------------------------------------------------------
bool FileSystemClass::SaveClustersToFile(ULONGLONG clusterId, DWORD numberOfClusters, WCHAR *fullPath)
{
	BYTE *clusterData;
	ULONGLONG totalBytesRead;
	DWORD clusterBufferSize;
	DWORD bufferSizeInClusters;
	DWORD clustersLeftToRead;
	DWORD clustersToRead;
	DWORD readSize;
	HANDLE fileHandle;
	DWORD bytesWritten;

	clustersLeftToRead = numberOfClusters;
	clusterBufferSize = (BytesPerCluster*numberOfClusters > (ULONGLONG)ReadBlockSize ? ReadBlockSize : BytesPerCluster * numberOfClusters);
	bufferSizeInClusters = clusterBufferSize / BytesPerCluster;
	clusterData = new BYTE[clusterBufferSize];

	// ������� �������� ����
	fileHandle = CreateFileW(
		fullPath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		delete[] clusterData;
		return false;
	}

	do
	{
		clustersToRead = (clustersLeftToRead > bufferSizeInClusters ? bufferSizeInClusters : clustersLeftToRead);
		readSize = ReadClustersByNumber(clusterId, clustersToRead, clusterData);

		if (readSize != clustersToRead * BytesPerCluster)
		{
			delete[] clusterData;
			return false;
		}

		clustersLeftToRead -= clustersToRead;
		clusterId += clustersToRead;

		if (!WriteFile(fileHandle, clusterData, readSize, &bytesWritten, NULL) || bytesWritten != readSize)
		{
			CloseHandle(fileHandle);
			delete[] clusterData;
			return false;
		}

	} while (clustersLeftToRead > 0);

	CloseHandle(fileHandle);
	delete[] clusterData;

	return true;
}
//---------------------------------------------------------------------------
//bool FileSystemClass::SaveClustersToFolder(ULONGLONG clusterId, DWORD numberOfClusters, WCHAR *targetFolder)
//{
//	bool result;
//	WCHAR fileName[24];
//	WCHAR *fullPath = new WCHAR[wcslen(targetFolder) + 24];
//
//	swprintf(fileName, L"\\%010Lu.dat", clusterId);
//	wcscpy(fullPath, targetFolder);
//	wcscat(fullPath, fileName);
//
//	result = SaveClustersToFile(clusterId, numberOfClusters, fullPath);
//
//	delete[] fullPath;
//
//	return result;
//}
////---------------------------------------------------------------------------
bool FileSystemClass::ClusterIsInBitmapCache(ULONGLONG clusterId)
{
	if (!ClusterBitmapCacheDefined) return false;
	if (clusterId >= ClusterBitmapCacheFirstCluster && clusterId <= ClusterBitmapCacheLastCluster) return true;

	return false;
}
//---------------------------------------------------------------------------
//bool FileSystemClass::ClusterIsAllocated(ULONGLONG clusterId)
//{
//	if (ClusterIsInBitmapCache(clusterId))
//	{
//		return ReadClusterStateFromCache(clusterId);
//	}
//	else
//	{
//		if (ClusterBitmapCacheDefined)
//		{
//			if (ClusterBitmapCacheInit(clusterId))
//			{
//				return ReadClusterStateFromCache(clusterId);
//			}
//		}
//		return ReadClusterStateFromDisk(clusterId);
//	}
//}
//---------------------------------------------------------------------------
//bool FileSystemClass::ReadClusterStateFromCache(ULONGLONG clusterId)
//{
//	// ����� ������� ������� ��������� �������� ����, ��� ������� ��������� � ����
//	return IsMarkedBitmap(ClusterBitmapCache, clusterId - ClusterBitmapCacheFirstCluster);
//}
//---------------------------------------------------------------------------
DWORD FileSystemClass::GetNumberOfFiles()
{
	// �� ��������� ��� �������� ������, �� �������������� �������� �����
	return 0;
}
//---------------------------------------------------------------------------
bool FileSystemClass::GetError()
{
	return Error;
}
//---------------------------------------------------------------------------
void FileSystemClass::SetError()
{
	Error = true;
}
//---------------------------------------------------------------------------
void FileSystemClass::ClearError()
{
	Error = false;
}
//---------------------------------------------------------------------------
bool FileSystemClass::CheckReadDataRange(const ULONGLONG realDataSize, const ULONGLONG allocatedDataSize, const DWORD bufferSize, const DWORD startReadVCN, const DWORD inVCNsToRead, ULONGLONG *outTotalBytesToRead, DWORD *outVCNsToRead, bool extendedRead)
{
	DWORD VCNsToRead = (inVCNsToRead == 0 ? (allocatedDataSize + BytesPerCluster - 1) / BytesPerCluster - startReadVCN : inVCNsToRead);

	ULONGLONG totalBytesToRead = (ULONGLONG)VCNsToRead*(ULONGLONG)BytesPerCluster; // ����� ����� ������, ���������� ���������� (��� ����� ������� ������)
	if (startReadVCN*BytesPerCluster + totalBytesToRead > realDataSize && !extendedRead) totalBytesToRead = realDataSize - (ULONGLONG)startReadVCN*(ULONGLONG)BytesPerCluster;

	if (startReadVCN*BytesPerCluster > realDataSize && !extendedRead)
	{
		*outTotalBytesToRead = totalBytesToRead;
		*outVCNsToRead = VCNsToRead;
		return false;
	}
	else if ((startReadVCN + VCNsToRead - 1)*BytesPerCluster > allocatedDataSize)
	{
		*outTotalBytesToRead = totalBytesToRead;
		*outVCNsToRead = VCNsToRead;
		return false;
	}

	if (bufferSize < BytesPerCluster)
	{
		// ����� ������� ���������
		*outTotalBytesToRead = totalBytesToRead;
		*outVCNsToRead = VCNsToRead;
		return false;
	}

	if (totalBytesToRead > bufferSize) VCNsToRead = bufferSize / BytesPerCluster;
	else VCNsToRead = (totalBytesToRead + BytesPerCluster - 1) / BytesPerCluster;

	*outTotalBytesToRead = totalBytesToRead;
	*outVCNsToRead = VCNsToRead;
	return true;
}
//---------------------------------------------------------------------------
Iterator<BinaryBlock>* FileSystemClass::GetClusterIterator(ULONGLONG startCluster, DWORD desiredClustersPerBlock, bool needOverlap, DWORD *outOverlapSize)
{
	return new FileSystemClusterIterator(this, startCluster, desiredClustersPerBlock, needOverlap, outOverlapSize);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
FileObjectClass::FileObjectClass()
{
	FilePathDefined = false;
	FileNameDefined = false;

	RecordIdDefined = false;
	RecordId = 0;
	RootDir = false;

	FileObjectIsUnpacked = false;

	FilePath = NULL;
	FileName = NULL;
	FilePathLength = 0;
	FileNameLength = 0;

	RecordBufferDefined = false;
	FileRecordSize = 0;
	RecordBuffer = NULL;
	RecordOffset = 0;

	TimeStampsDefined = false;
	TimeFileCreate.QuadPart = 0L;
	TimeFileModify.QuadPart = 0L;
	TimeFileAccess.QuadPart = 0L;
	TimeRecordModify.QuadPart = 0L;

	DataStreamsDefined = false;
	//NumberOfStreams = 0;
	//FirstDataStream = NULL;
	//LastDataStream = NULL;
}
//---------------------------------------------------------------------------
//FileObjectClass::~FileObjectClass()
//{
//	if (RecordBufferDefined)
//	{
//		delete[] RecordBuffer;
//		RecordBuffer = NULL;
//		RecordBufferDefined = false;
//	}
//
//	ClearFileName();
//	ClearFilePath();
//	//ClearDataStreams();
//
//	// �������� ��������
//	while (Context.size() > 0)
//	{
//		PFileContext currentContext = Context.top();
//		delete currentContext.second;
//		delete currentContext.first;
//		Context.pop();
//	}
//}
//---------------------------------------------------------------------------
void FileObjectClass::ClearFileName()
{
	if (FileName != NULL)
	{
		delete[] FileName;
		FileName = NULL;
	}

	FileNameLength = 0;
	FileNameDefined = false;
}
//---------------------------------------------------------------------------
void FileObjectClass::ClearFilePath()
{
	if (FilePath != NULL)
	{
		delete[] FilePath;
		FilePath = NULL;
	}

	FilePathLength = 0;
	FilePathDefined = false;
}
//---------------------------------------------------------------------------
//LONGLONG FileObjectClass::GetRecordId()
//{
//	if (!RecordIdDefined) return -1;
//	return RecordId;
//}
////---------------------------------------------------------------------------
//LONGLONG FileObjectClass::GetBitmapId()
//{
//	// �������������, ������� ������������ ��� ��������� ����� � ������� �����.
//	// ��� ����������� �������� ������ ��������� � ���������� ��������������� �����.
//	return GetRecordId();
//}
////---------------------------------------------------------------------------
//LONGLONG FileObjectClass::GetParentId()
//{
//	return -1; // ��������
//}
////---------------------------------------------------------------------------
//LONGLONG FileObjectClass::GetBaseId()
//{
//	return -1; // ��������
//}
//---------------------------------------------------------------------------
DWORD FileObjectClass::GetFileRecordSize()
{
	return FileRecordSize;
}
//---------------------------------------------------------------------------
DWORD FileObjectClass::GetFileRecord(BYTE *dataBuffer)
{
	if (!RecordBufferDefined) return 0;
	if (dataBuffer != NULL) memcpy(dataBuffer, RecordBuffer, FileRecordSize);
	return FileRecordSize;
}
//---------------------------------------------------------------------------
LONGLONG FileObjectClass::GetRecordOffset()
{
	if (!RecordBufferDefined) return -1;
	return RecordOffset;
}
//---------------------------------------------------------------------------
//WORD FileObjectClass::GetFileNameLength()
//{
//	if (!FileNameDefined)
//	{
//		return GetFileName();
//	}
//	else
//	{
//		return FileNameLength;
//	}
//}
////---------------------------------------------------------------------------
//WORD FileObjectClass::GetFilePathLength()
//{
//	if (!FilePathDefined)
//	{
//		return GetFilePath();
//	}
//	else
//	{
//		return FilePathLength;
//	}
//}
////---------------------------------------------------------------------------
//WORD FileObjectClass::GetFullPathLength()
//{
//	return GetFullPath();
//}
////---------------------------------------------------------------------------
//WORD FileObjectClass::GetFileName(WCHAR *targetBuffer)
//{
//	if (!FileNameDefined) return 0;
//	//if(targetBuffer != NULL) wcsncpy(targetBuffer,FileName,FileNameLength+1);
//	if (targetBuffer != NULL)
//	{
//		memcpy((BYTE*)targetBuffer, (BYTE*)FileName, 2 * FileNameLength);
//		targetBuffer[FileNameLength] = 0;
//	}
//	return FileNameLength;
//}
////---------------------------------------------------------------------------
//const WCHAR *FileObjectClass::GetFileNamePtr()
//{
//	// ���������, ������� �� ��� �����
//	if (!FileNameDefined)
//	{
//		// ���� �� �������, ���������� �������
//		if (GetFileName() == 0) return NULL;
//	}
//	return FileName;
//}
////---------------------------------------------------------------------------
//wstring FileObjectClass::GetFileNameStr()
//{
//	// ���������, ������� �� ��� �����
//	if (!FileNameDefined)
//	{
//		// ���� �� �������, ���������� �������
//		if (GetFileName() == 0) return wstring();
//	}
//
//	return wstring(FileName, FileNameLength);
//}
////---------------------------------------------------------------------------
//WORD FileObjectClass::GetFilePath(WCHAR *targetBuffer)
//{
//	if (!FilePathDefined) return 0;
//	if (targetBuffer != NULL)
//	{
//		memcpy((BYTE*)targetBuffer, (BYTE*)FilePath, 2 * FilePathLength);
//		targetBuffer[FilePathLength] = 0;
//	}
//	return FilePathLength;
//}
////---------------------------------------------------------------------------
//const WCHAR *FileObjectClass::GetFilePathPtr()
//{
//	if (!FilePathDefined)
//	{
//		if (GetFilePath() == 0) return NULL;
//	}
//	return FilePath;
//}
////---------------------------------------------------------------------------
//wstring FileObjectClass::GetFilePathStr()
//{
//	// ���������, ������� �� ��� �����
//	if (!FilePathDefined)
//	{
//		// ���� �� �������, ���������� �������
//		if (GetFilePath() == 0) return wstring();
//	}
//
//	return wstring(FilePath, FilePathLength);
//}
////---------------------------------------------------------------------------
////WORD FileObjectClass::GetFullPath(WCHAR *targetBuffer)
////{
////	WORD outStrLength;
////
////	if (!FileNameDefined) GetFileName();
////	if (!FilePathDefined) GetFilePath();
////
////	outStrLength = 0;
////	if (FilePathDefined && FilePathLength > 0)
////	{
////		if (RootDir)
////		{
////			if (targetBuffer != NULL) wcscpy(targetBuffer, L"\\");
////			outStrLength += 1;
////		}
////		else
////		{
////			if (targetBuffer != NULL) wcsncpy(targetBuffer, FilePath, FilePathLength + 1);
////			if (targetBuffer != NULL) wcscat(targetBuffer, L"\\");
////			outStrLength += FilePathLength + 1;
////		}
////	}
////	else
////	{
////		if (targetBuffer != NULL) wcscpy(targetBuffer, L"");
////	}
////
////	if (FileNameDefined && FileNameLength > 0)
////	{
////		if (targetBuffer != NULL) wcscat(targetBuffer, FileName);
////		outStrLength += FileNameLength;
////	}
////
////	return outStrLength;
////}
////---------------------------------------------------------------------------
//wstring FileObjectClass::GetFullPathStr()
//{
//	WCHAR *fullPath;
//	WORD fullPathLength;
//	wstring outStr;
//
//	// ���������� ��������� ����� ������
//	fullPathLength = GetFullPathLength();
//	if (fullPathLength == 0) return L"";
//
//	// �������� ������
//	fullPath = new WCHAR[fullPathLength + 1];
//
//	// ������� ������ ����
//	if (GetFullPath(fullPath) == fullPathLength)
//	{
//		// ���� ������� ������
//		outStr = wstring(fullPath);
//	}
//	else
//	{
//		// ������ ������!
//		outStr = wstring();
//	}
//
//	// �������� ������
//	delete[] fullPath;
//	return outStr;
//}
////---------------------------------------------------------------------------
//bool FileObjectClass::SameName(const WCHAR *fileName)
//{
//	WORD fileNameLength;
//	const WCHAR *fileNamePtr;
//
//	fileNameLength = GetFileNameLength();
//	fileNamePtr = GetFileNamePtr();
//
//	if (fileNamePtr == NULL) return false;
//
//	if (wcslen(fileName) != DWORD(fileNameLength)) return false;
//	//if(wcsncmpi(fileName,fileNamePtr,fileNameLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, fileName, fileNameLength, fileNamePtr, fileNameLength) != 2) return false;
//
//	return true;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::SameFiles(const WCHAR *inFullPath)
//// ��������� ���� ������ �� ������� ����
//{
//	WORD filePathLength;
//	WORD fileNameLength;
//	const WCHAR *filePathPtr;
//	const WCHAR *fileNamePtr;
//	WORD inFullPathLength = wcslen(inFullPath);
//
//	if (inFullPath == NULL) return false;
//
//	// ���������, �� �������� �� ������� ���� �������� ���������
//	if (IsRoot() && inFullPathLength == ROOT_DIR_LENGTH)
//	{
//		if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, inFullPath, inFullPathLength, ROOT_DIR, ROOT_DIR_LENGTH) == 2) return true;
//	}
//
//	filePathLength = GetFilePathLength();
//	filePathPtr = GetFilePathPtr();
//	if (filePathPtr == NULL) return false;
//
//	fileNameLength = GetFileNameLength();
//	fileNamePtr = GetFileNamePtr();
//	if (fileNamePtr == NULL) return false;
//
//	if (inFullPathLength != WORD(filePathLength + fileNameLength + 1)) return false;
//
//	//if(wcsncmpi(fullPath,filePathPtr,filePathLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, inFullPath, filePathLength, filePathPtr, filePathLength) != 2) return false;
//
//	//if(wcsncmpi(&fullPath[filePathLength+1],fileNamePtr,fileNameLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, &inFullPath[filePathLength + 1], fileNameLength, fileNamePtr, fileNameLength) != 2) return false;
//
//	return true;
//}
////---------------------------------------------------------------------------
//bool FileObjectClass::SameFiles(FileObjectClass *fileObject)
//{
//	if (!FileNameDefined) return false;
//
//	//if(_wcsicmp(FileName,fileObject->GetFileNamePtr()) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, FileName, -1, fileObject->GetFileNamePtr(), -1) != CSTR_EQUAL) return false;
//
//	//if(_wcsicmp(FilePath,fileObject->GetFilePathPtr()) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, FilePath, -1, fileObject->GetFilePathPtr(), -1) != CSTR_EQUAL) return false;
//
//	return true;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::SamePath(const WCHAR *fullPath)
//// ��������� ����� ���� ������
//{
//	WORD filePathLength=0;
//	WORD targetFilePathLength;
//	const WCHAR *filePathPtr;
//
//	if (fullPath == NULL) return false;
//
//	//filePathLength = GetFilePathLength();
//	filePathPtr = GetFilePathPtr();
//
//	if (filePathLength == 0 || filePathPtr == NULL) return false;
//
//	targetFilePathLength = wcsrchr(fullPath, '\\') - fullPath;
//
//	if (targetFilePathLength != filePathLength) return false;
//	//if(wcsncmpi(fullPath,filePathPtr,targetFilePathLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, fullPath, targetFilePathLength, filePathPtr, targetFilePathLength) != 2) return false;
//
//	return true;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::Contains(const WCHAR *fullPath)
//// �������� ����, ���������� �� �������� ���� � ������� �������� (�� ����)
//{
//	WORD targetFilePathLength;
//	WORD filePathLength;
//	WORD fileNameLength;
//	const WCHAR *filePathPtr;
//	const WCHAR *fileNamePtr;
//
//	if (fullPath == NULL) return false;
//	if (!IsDir()) return false; // ��� �� �������, ������ �� ��������� �� �����
//
//	targetFilePathLength = wcsrchr(fullPath, '\\') - fullPath;
//
//	filePathLength = GetFilePathLength();
//	filePathPtr = GetFilePathPtr();
//	if (filePathPtr == NULL) return false;
//
//	fileNameLength = GetFileNameLength();
//	fileNamePtr = GetFileNamePtr();
//	if (fileNamePtr == NULL) return false;
//
//	if (targetFilePathLength != filePathLength + fileNameLength + 1) return false;
//
//	//if(wcsncmpi(fullPath,filePathPtr,filePathLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, fullPath, filePathLength, filePathPtr, filePathLength) != 0) return false;
//
//	//if(wcsncmpi(&fullPath[filePathLength+1],fileNamePtr,fileNameLength) != 0) return false;
//	if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, &fullPath[filePathLength + 1], fileNameLength, fileNamePtr, fileNameLength) != 0) return false;
//
//	return true;
//}
//---------------------------------------------------------------------------
LARGE_INTEGER FileObjectClass::GetTimeFileCreate()
{
	LARGE_INTEGER zeroTime;
	zeroTime.QuadPart = 0L;

	if (!TimeStampsDefined)
	{
		if (!ReadTimeStamps()) return zeroTime;
	}

	return TimeFileCreate;
}
//---------------------------------------------------------------------------
LARGE_INTEGER FileObjectClass::GetTimeFileModify()
{
	LARGE_INTEGER zeroTime;
	zeroTime.QuadPart = 0L;

	if (!TimeStampsDefined)
	{
		if (!ReadTimeStamps()) return zeroTime;
	}

	return TimeFileModify;
}
//---------------------------------------------------------------------------
LARGE_INTEGER FileObjectClass::GetTimeFileAccess()
{
	LARGE_INTEGER zeroTime;
	zeroTime.QuadPart = 0L;

	if (!TimeStampsDefined)
	{
		if (!ReadTimeStamps()) return zeroTime;
	}

	return TimeFileAccess;
}
//---------------------------------------------------------------------------
LARGE_INTEGER FileObjectClass::GetTimeRecordModify()
{
	LARGE_INTEGER zeroTime;
	zeroTime.QuadPart = 0L;

	if (!TimeStampsDefined)
	{
		if (!ReadTimeStamps()) return zeroTime;
	}

	return TimeRecordModify;
}
//---------------------------------------------------------------------------
//WORD FileObjectClass::GetNumberOfStreams()
//{
//	return DataStreams.size();
//}
//---------------------------------------------------------------------------
//void FileObjectClass::AddDataStream(DataStreamClass *newDataStream)
//{
//	if (newDataStream->GetBytesPerVCN() != 0)
//	{
//		DataStreams.push_back(newDataStream);
//		newDataStream->SetIndex(DataStreams.size() - 1);
//	}
//}
//---------------------------------------------------------------------------
//void FileObjectClass::ClearDataStreams()
//{
//	for (PIterator<DataStreamClass*> i = GetDataStreamIterator(); !i->IsDone(); i->Next())
//	{
//		delete i->GetCurrent();
//	}
//
//	DataStreams.clear();
//	DataStreamsDefined = false;
//}
//---------------------------------------------------------------------------
//Iterator<DataStreamClass*>* FileObjectClass::GetDataStreamIterator()
//{
//	return new IteratorAdapter<list<DataStreamClass*>, DataStreamClass*>(&DataStreams);
//}
//---------------------------------------------------------------------------
//class DataStreamClass *FileObjectClass::GetFirstDataStream()
//{
//	if (DataStreams.size() > 0)
//	{
//		return DataStreams.front();
//	}
//	else
//	{
//		return NULL;
//	}
//}
//---------------------------------------------------------------------------
//list< pair<int, wstring> > __fastcall FileObjectClass::GetFileStreamNames()
//{
//	list< pair<int, wstring> > outList;
//
//	for (PIterator<DataStreamClass*> i = GetDataStreamIterator(); !i->IsDone(); i->Next())
//	{
//		DataStreamClass *currentStream = i->GetCurrent();
//
//		// ������� ������������� ������
//		int streamId = currentStream->GetId();
//
//		// ���������� ����� ����� ���������� ������
//		WORD streamNameLength = currentStream->GetStreamNameLength();
//
//		if (streamNameLength > 0)
//		{
//			// �������� ������
//			WCHAR *streamName = new WCHAR[streamNameLength + 1];
//
//			if (currentStream->GetStreamName(streamName) == streamNameLength)
//			{
//				// ������� ����� ������
//				outList.push_back(pair<int, wstring>(streamId, streamName));
//			}
//			else
//			{
//				outList.push_back(pair<int, wstring>(streamId, wstring(L"������!")));
//			}
//
//			// �������� ������
//			delete[] streamName;
//		}
//		else
//		{
//			// ���������� ����� ������
//			outList.push_back(pair<int, wstring>(streamId, wstring()));
//		}
//	}
//
//	return outList;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::IsRoot()
//{
//	return RootDir;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::IsMarkedInBitmap(const BYTE *fileBitmap)
//{
//	if (!RecordIdDefined) return false;
//	if (fileBitmap == NULL) return false;
//	else return IsMarkedBitmap(fileBitmap, RecordId);
//}
//---------------------------------------------------------------------------
//void FileObjectClass::MarkFileInBitmap(BYTE *fileBitmap)
//{
//	if (fileBitmap == NULL) return;
//	if (RecordIdDefined) MarkBitmap(RecordId, fileBitmap);
//}
//---------------------------------------------------------------------------
//WORD FileObjectClass::GetCommonFlags()
//{
//	WORD commonFlags = 0;
//
//	if (IsUsed()) commonFlags |= Used_CommonFlag;
//	if (IsDir()) commonFlags |= Dir_CommonFlag;
//
//	return commonFlags;
//}
//---------------------------------------------------------------------------
//DWORD FileObjectClass::GetStdInfoFlags()
//{
//	return 0;
//}
//---------------------------------------------------------------------------
//bool FileObjectClass::SaveTo(WCHAR *targetFolder, const WCHAR *fileName, bool absolutePath, bool includeSubdirs, bool copyFileTime, bool addRecordId)
//{
//	bool result = true;
//	WCHAR fileRecordIdString[24];
//
//	// ���������� ����� ����� ������������ �����
//	WORD fileNameLength;
//	if (fileName != NULL)
//	{
//		// ���� ��� ���� �������, ������������ ���
//		fileNameLength = wcslen(fileName);
//	}
//	else
//	{
//		// ���� ���, ��������������� ����������� �� �������� �������
//		fileNameLength = GetFileNameLength();
//	}
//
//	WCHAR *targetPath = NULL;
//	WORD targetFolderLength = wcslen(targetFolder);
//	if (absolutePath)
//	{
//		// ����� ���������� ������� ���� �����
//		WORD filePathLength = GetFullPathLength();
//		targetPath = new WCHAR[filePathLength + targetFolderLength + 4];
//
//		if (targetFolder[targetFolderLength - 1] == L'\\')
//		{
//			wcsncpy(targetPath, targetFolder, targetFolderLength - 1);
//			targetPath[targetFolderLength - 1] = 0;
//		}
//		else
//		{
//			wcscpy(targetPath, targetFolder);
//		}
//
//		if (filePathLength > 0)
//		{
//			WCHAR *filePath = new WCHAR[filePathLength + 1];
//			GetFilePath(filePath);
//
//			// ���������� �������� ��� ������� '|' �� '\\',
//			// ����� ��������� �������� ��� ��������
//			//ReplaceCharW(filePath, filePathLength, L'|', L'\\');
//
//			if (wcscmp(filePath, ROOT_DIR) != 0)
//			{
//				wcscat(targetPath, &filePath[ROOT_DIR_LENGTH]);
//			}
//
//			delete[] filePath;
//		}
//
//		// ���������, ���������� �� ��������� ����
//		if (!ForceDirectory(targetPath))
//		{
//			delete[] targetPath;
//			return false;
//		}
//	}
//	else
//	{
//		targetPath = new WCHAR[MAX_PATH * 2 + 256 + 4];
//
//		// ���������, ���������� �� ��������� ����
//		if (!ForceDirectory(targetFolder))
//		{
//			return false;
//		}
//
//		//targetPath = new WCHAR[targetFolderLength+fileNameLength+8+sizeof(fileRecordIdString)/sizeof(WCHAR)+2];
//		wcscpy(targetPath, targetFolder);
//	}
//
//	if (IsDir())
//	{
//		PIterator<FileObjectClass*> dirIterator = GetDirectoryFileIterator();
//
//		DWORD nextTargetFolderLength = wcslen(targetPath) + fileNameLength + 16 + 2; // 16 �������� - ����� �� �������������
//		WCHAR *nextTargetFolder = new WCHAR[nextTargetFolderLength];
//		wcscpy(nextTargetFolder, targetPath);
//		wcscat(nextTargetFolder, L"\\");
//
//		if (fileName != NULL)
//		{
//			wcscat(nextTargetFolder, fileName);
//		}
//		else if (fileNameLength > 0)
//		{
//			wcscat(nextTargetFolder, GetFileNamePtr());
//		}
//		else
//		{
//			WCHAR *tempFileName = new WCHAR[16];
//			swprintf(tempFileName, L"Folder.%08Ld", GetRecordId());
//			wcscat(nextTargetFolder, tempFileName);
//			delete[] tempFileName;
//		}
//
//		if (ForceDirectory(nextTargetFolder))
//		{
//			bool nextIncludeSubdirs = includeSubdirs;
//			bool nextAbsolutePath = false;
//			if (!SaveDir(dirIterator, nextTargetFolder, nextAbsolutePath, nextIncludeSubdirs, copyFileTime)) result = false;
//		}
//		else
//		{
//			result = false;
//		}
//
//		delete[] nextTargetFolder;
//	}
//
//	WORD numberOfStreams = GetNumberOfStreams();
//	if (numberOfStreams > 0)
//	{
//		// ������������ ������ ���� � �����
//		wcscat(targetPath, L"\\");
//
//		// �������� ������������� �����
//		if (addRecordId && fileNameLength > 0)
//		{
//			swprintf(fileRecordIdString, L"%08Ld\.", GetRecordId());
//			wcscat(targetPath, fileRecordIdString);
//		}
//		else if (fileNameLength == 0)
//		{
//			// ����� ��� ����� ����������, ��� �������� �������������
//			swprintf(fileRecordIdString, L"File.%08Ld", GetRecordId());
//			wcscat(targetPath, fileRecordIdString);
//		}
//
//		if (fileName != NULL)
//		{
//			wcscat(targetPath, fileName);
//		}
//		else if (fileNameLength > 0)
//		{
//			wcscat(targetPath, GetFileNamePtr());
//		}
//
//		WORD targetPathLength = wcslen(targetPath);
//		WORD m = 0;
//		for (PIterator<DataStreamClass*> i = GetDataStreamIterator(); !i->IsDone(); i->Next(), m++)
//		{
//			targetPath[targetPathLength] = 0;
//			DataStreamClass *currentStream = i->GetCurrent();
//
//			WORD streamNameLength = currentStream->GetStreamNameLength();
//
//			if (streamNameLength > 0)
//			{
//				WCHAR *streamName = new WCHAR[streamNameLength + 2];
//				streamName[0] = L'.';
//				currentStream->GetStreamName(&streamName[1]);
//
//				wcscpy(&targetPath[targetPathLength], streamName);
//				targetPath[targetPathLength + streamNameLength + 1] = 0;
//				delete[] streamName;
//			}
//			else if (numberOfStreams > 1 && m > 0)
//			{
//				// ������������� ��������, ����� ���������� ������� ��� ������ ������,
//				// ������ ����� ����� �������������� ���������� �����
//
//				WCHAR streamNumberString[6];
//				swprintf(streamNumberString, L".%hu", m);
//				wcscpy(&targetPath[targetPathLength], streamNumberString);
//				targetPath[targetPathLength + streamNameLength + wcslen(streamNumberString)] = 0;
//			}
//
//			if (!currentStream->SaveDataToFile(targetPath))
//			{
//				delete[] targetPath;
//				return false;
//			}
//		}
//	}
//
//	delete[] targetPath;
//	return result;
//}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//DataStreamClass::DataStreamClass()
//{
//	Id = 0;
//	Index = 0;
//
//	StreamNameLength = 0;
//	StreamName = NULL;
//
//	RealDataSize = 0;
//	AllocatedDataSize = 0;
//
//	BytesPerVCN = 0;
//}
//---------------------------------------------------------------------------
//bool DataStreamClass::IsNormalDataStream() const
//{
//	return true;
//}
////---------------------------------------------------------------------------
//DataStreamClass::~DataStreamClass()
//{
//	if (StreamNameLength > 0) delete[] StreamName;
//	StreamName = NULL;
//	StreamNameLength = 0;
//}
////---------------------------------------------------------------------------
//WORD DataStreamClass::GetStreamNameLength() const
//{
//	return StreamNameLength;
//}
////---------------------------------------------------------------------------
////WORD DataStreamClass::GetStreamName(WCHAR *targetBuffer) const
////{
////	if (StreamNameLength == 0) return 0;
////
////	if (targetBuffer != NULL) wcsncpy(targetBuffer, StreamName, StreamNameLength + 1);
////	return StreamNameLength;
////}
////---------------------------------------------------------------------------
//const WCHAR *DataStreamClass::GetStreamNamePtr() const
//{
//	if (StreamNameLength == 0) return NULL;
//	else return StreamName;
//}
////---------------------------------------------------------------------------
//wstring DataStreamClass::GetStreamNameStr() const
//{
//	if (StreamNameLength == 0) return wstring();
//	else return wstring(StreamName, StreamNameLength);
//}
////---------------------------------------------------------------------------
//WORD DataStreamClass::GetId() const
//{
//	return Id;
//}
////---------------------------------------------------------------------------
//void DataStreamClass::SetId(WORD newId)
//{
//	Id = newId;
//}
////---------------------------------------------------------------------------
//WORD DataStreamClass::GetIndex() const
//{
//	return Index;
//}
////---------------------------------------------------------------------------
//void DataStreamClass::SetIndex(WORD newIndex)
//{
//	Index = newIndex;
//}
////---------------------------------------------------------------------------
//ULONGLONG DataStreamClass::GetRealDataSize() const
//{
//	return RealDataSize;
//}
////---------------------------------------------------------------------------
//ULONGLONG DataStreamClass::GetAllocatedDataSize() const
//{
//	return AllocatedDataSize;
//}
//---------------------------------------------------------------------------
//DWORD DataStreamClass::GetBytesPerVCN() const
//{
//	return BytesPerVCN;
//}
////---------------------------------------------------------------------------
//DWORD DataStreamClass::GetMinBufferSize() const
//{
//	return BytesPerVCN;
//}
////---------------------------------------------------------------------------
//bool DataStreamClass::SaveDataToFile(WCHAR *fileName, bool forceCreate)
//{
//	HANDLE fileHandle;
//	DWORD minBufferSize, bufferSize;
//	BYTE *dataBuffer;
//	DWORD readSize;
//	ULONGLONG totalReadSize;
//	ULONGLONG leftToRead;
//	DWORD startVCN;
//	DWORD bytesToWrite;
//	DWORD bytesWritten;
//	ULARGE_INTEGER fileSize;
//
//	// �������� �����!!! ����� � ������ ����������� � Windows ��������.
//	// �����������, ��� ������ ���� ���� ���!
//
//	// ����������� �� ��������� ��� ������ ���������� �������� �������� (text/plain)
//	//ReplaceCharW(fileName, 0, L'/', L'_');
//
//	// ���������, ���� �� ���� ��� ������� � ����� ������
//	if (!forceCreate)
//	{
//		DWORD fileAttributes = GetFileAttributesW(fileName);
//		if (fileAttributes != INVALID_FILE_ATTRIBUTES)
//		{
//			if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//			{
//				return true;
//			}
//			else
//			{
//				fileHandle = CreateFileW(
//					fileName,
//					GENERIC_READ,
//					0,
//					NULL,
//					OPEN_EXISTING,
//					//FILE_ATTRIBUTE_NORMAL,
//					FILE_FLAG_BACKUP_SEMANTICS,
//					NULL
//				);
//
//				if (fileHandle != INVALID_HANDLE_VALUE)
//				{
//					fileSize.QuadPart = 0L;
//					fileSize.LowPart = GetFileSize(fileHandle, &fileSize.HighPart);
//					CloseHandle(fileHandle);
//
//					if (fileSize.QuadPart == RealDataSize) return true;
//				}
//			}
//		}
//	}
//
//	// ������� ����� ����
//	fileHandle = CreateFileW(
//		fileName,
//		GENERIC_WRITE,
//		0,
//		NULL,
//		CREATE_ALWAYS,
//		FILE_ATTRIBUTE_NORMAL,
//		NULL
//	);
//
//	if (fileHandle == INVALID_HANDLE_VALUE) return false;
//
//	if (RealDataSize == 0)
//	{
//		// ����� ������ ����� ������� ������
//		CloseHandle(fileHandle);
//		return true;
//	}
//
//	// �������� ������ ��� ������
//	minBufferSize = GetMinBufferSize();
//	bufferSize = ((ReadBlockSize + minBufferSize - 1) / minBufferSize)*minBufferSize;
//	dataBuffer = new BYTE[bufferSize];
//	totalReadSize = 0;
//
//	do
//	{
//		startVCN = totalReadSize / minBufferSize;
//		readSize = ReadData(dataBuffer, bufferSize, &leftToRead, startVCN);
//		if (readSize == 0 || (leftToRead != 0 && readSize < bufferSize))
//		{
//			// ������ ������ (����� ���������� �� ���������)
//			delete[] dataBuffer;
//			CloseHandle(fileHandle);
//			return false;
//		}
//
//		bytesToWrite = (totalReadSize + readSize > RealDataSize ? RealDataSize - totalReadSize : readSize);
//		// �������� � ���� ���������� ������
//		if (!WriteFile(fileHandle, dataBuffer, bytesToWrite, &bytesWritten, NULL) || (bytesWritten != bytesToWrite))
//		{
//			// ������ ������ � ����
//			delete[] dataBuffer;
//			CloseHandle(fileHandle);
//			return false;
//		}
//
//		totalReadSize += readSize;
//
//	} while (totalReadSize < RealDataSize);
//
//	// ������� ����
//	CloseHandle(fileHandle);
//
//	delete[] dataBuffer;
//	return true;
//}
//---------------------------------------------------------------------------
//Iterator<BinaryBlock>* DataStreamClass::GetIterator(ULONGLONG startVCN, DWORD desiredVCNsPerBlock, bool needOverlap, DWORD *outOverlapSize)
//{
//	return new DataStreamVCNIterator(this, startVCN, desiredVCNsPerBlock, needOverlap, outOverlapSize);
//}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BinaryBlockIterator::BinaryBlockIterator(
	ULONGLONG startIndex,
	DWORD bytesPerCluster,
	ULONGLONG numberOfClusters,
	DWORD minReadBlock,
	DWORD desiredClustersPerReadBlock,
	bool needOverlap,
	DWORD *outOverlapSize
)
{
	StartIndex = startIndex;
	CurrentIndex = startIndex;
	BytesPerCluster = bytesPerCluster;
	NumberOfClusters = numberOfClusters;
	DWORD clusterssPerMinBuffer = minReadBlock / bytesPerCluster;

	// ������ ����� ������ ���� ������ minBufferSize
	if (desiredClustersPerReadBlock == 0)
	{
		ClustersPerReadBlock = clusterssPerMinBuffer;
	}
	else
	{
		ClustersPerReadBlock = (desiredClustersPerReadBlock + clusterssPerMinBuffer - 1) / clusterssPerMinBuffer * clusterssPerMinBuffer;
		ClustersPerReadBlock = (ClustersPerReadBlock > numberOfClusters ? numberOfClusters : ClustersPerReadBlock);
	}

	// ��� �� ����� ���� ������ minBufferSize
	if (!needOverlap)
	{
		// ��� �������� � ����� �����
		StepSizeInClusters = ClustersPerReadBlock;
	}
	else
	{
		// �������� ��������� ������ ���� �������������
		if (ClustersPerReadBlock == clusterssPerMinBuffer) // ���������, �� �������� �� ������ ������ ������� ���������
		{
			// ���������� ��������� ����� ������������ �����, ����� ���������� �� ��������
			ClustersPerReadBlock += clusterssPerMinBuffer;
			StepSizeInClusters = clusterssPerMinBuffer;
		}
		else
		{
			StepSizeInClusters = ClustersPerReadBlock - clusterssPerMinBuffer;
		}
	}

	ReadBlockSizeInBytes = BytesPerCluster * ClustersPerReadBlock;

	if (outOverlapSize != NULL) *outOverlapSize = (ClustersPerReadBlock - StepSizeInClusters)*bytesPerCluster;
}
//---------------------------------------------------------------------------
FileSystemClusterIterator::FileSystemClusterIterator(FileSystemClass *fileSystem, ULONGLONG startCluster, DWORD desiredVCNsPerBlock, bool needOverlap, DWORD *outOverlapSize)
	: BinaryBlockIterator(
		startCluster,
		fileSystem->GetBytesPerCluster(),
		fileSystem->GetTotalClusters(),
		fileSystem->GetBytesPerCluster(),
		desiredVCNsPerBlock,
		needOverlap,
		outOverlapSize
	)
{
	FileSystem = fileSystem;
}
//---------------------------------------------------------------------------
BinaryBlock FileSystemClusterIterator::GetCurrent() const
{
	BinaryBlock outBlock;
	outBlock.resize(ReadBlockSizeInBytes);

	ULONGLONG leftToRead;
	DWORD readSize = FileSystem->ReadClustersByNumber(CurrentIndex, ClustersPerReadBlock, &outBlock.front());
	outBlock.resize(readSize);

	return outBlock;
}
//---------------------------------------------------------------------------
////---------------------------------------------------------------------------
//bool SaveDir(PIterator<FileObjectClass*>& dirIterator, WCHAR *targetFolder, bool recoverPath, bool includeSubdirs, bool copyFileTime)
//{
//	DWORD nextTargetFolderLength;
//	WCHAR *nextTargetFolder;
//	bool result = true;
//
//	for (dirIterator->First(); !dirIterator->IsDone(); dirIterator->Next())
//	{
//		FileObjectClass *currentFile = dirIterator->GetCurrent();
//		if (currentFile == NULL) return false;
//
//		if (!currentFile->SameName(L".") && !currentFile->SameName(L".."))
//		{
//			if (!currentFile->IsDir() || (currentFile->IsDir() && includeSubdirs))
//			{
//				if (!currentFile->SaveTo(targetFolder, NULL, recoverPath, includeSubdirs, copyFileTime)) result = false;
//			}
//		}
//
//		delete currentFile;
//		currentFile = NULL;
//	}
//
//	return result;
//}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//bool ForceDirectory(const WCHAR *dirPath)
//{
//	const WCHAR *tempDir;
//	DWORD tempDirLength;
//	const WCHAR *readDir;
//	DWORD readDirLength;
//	DWORD fileAttributes;
//
//	WCHAR *pFirstDelimiter;
//	int firstDelimiter;
//
//	DWORD fullPathLength = wcslen(dirPath);
//	WCHAR *openFileName = new WCHAR[fullPathLength + 1];
//
//	// ��������� ��� ������� ����� ������� (����� ������� ���� ��� ��������� �������� � ������� ������)
//	tempDir = &dirPath[2];
//	tempDirLength = fullPathLength - 2;
//
//	readDir = dirPath;
//	readDirLength = 2;
//
//	while (true)
//	{
//		tempDir = &tempDir[1];
//		tempDirLength--;
//		readDirLength++;
//
//		pFirstDelimiter = wcschr(tempDir, '\\');
//
//		if (pFirstDelimiter != NULL)
//		{
//			firstDelimiter = (pFirstDelimiter - tempDir);
//			tempDir = &tempDir[firstDelimiter];
//			tempDirLength = tempDirLength - firstDelimiter;
//			readDirLength += firstDelimiter;
//		}
//		else
//		{
//			tempDirLength = 0;
//			readDirLength = fullPathLength;
//		}
//
//		// � currentDir ���������� ��� �����������, ������� ���������� ���������� �������.
//		// � readDir ���������� ������ ���� ����� ��������.
//
//		// ���������� ������� ����
//		wcsncpy(openFileName, readDir, readDirLength);
//		openFileName[readDirLength] = 0;
//		fileAttributes = GetFileAttributesW(openFileName);
//
//		if (fileAttributes == 0xFFFFFFFF)
//		{
//			// ��� ������ ��������, ���� �������
//			if (!CreateDirectoryW(openFileName, NULL))
//			{
//				delete[] openFileName;
//				return false;
//			}
//		}
//
//		if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//		{
//			// ���� ����� �������, ����������
//			if (readDirLength == fullPathLength) break;
//			else continue;
//		}
//	}
//
//	delete[] openFileName;
//	return true;
//}
//---------------------------------------------------------------------------
bool GetExtentReadInfo(
	DWORD startReadVCN,    // ��������� VCN, � �������� ���������� �������� ������
	DWORD totalVCNsToRead, // ����� ���������� ���������, ������� ���������� �������
	DWORD VCNsToRead,      // ���������� ���������, ������� �������� �������
	DWORD currentReadVCN,  // ������� ������� � ����� (��������� VCN)
	DWORD extentStartVCN,  // ��������� VCN ��������
	ULONGLONG extentStartCluster, // ��������� ���������� ������� ��������
	DWORD extentSize,      // ������ �������� � ���������
	ULONGLONG *startReadCluster, // ���������� �������, � �������� ���������� ������ ������
	DWORD *numberOfClustersToRead // ���������� ���������� ���������, ������� ���������� �������
)
{
	bool readMode = false;

	if (currentReadVCN >= startReadVCN && currentReadVCN >= extentStartVCN && extentStartVCN + extentSize > currentReadVCN)
	{
		// ���������� ������ ������ ������ � ������� ��������
		readMode = true;
		*startReadCluster = currentReadVCN - extentStartVCN + extentStartCluster;

		if (currentReadVCN + VCNsToRead > extentStartVCN + extentSize)
		{
			// ������� �� ����� ��������
			*numberOfClustersToRead = extentStartVCN + extentSize - currentReadVCN;
		}
		else
		{
			*numberOfClustersToRead = VCNsToRead;
		}
	}
	else if (currentReadVCN < startReadVCN && extentStartVCN < startReadVCN && extentStartVCN + extentSize > startReadVCN)
	{
		readMode = true;
		*startReadCluster = startReadVCN - extentStartVCN + extentStartCluster;

		if (startReadVCN + totalVCNsToRead > extentStartVCN + extentSize)
		{
			// ������� �� ����� ��������
			*numberOfClustersToRead = extentStartVCN + extentSize - startReadVCN;
		}
		else
		{
			*numberOfClustersToRead = totalVCNsToRead;
		}
	}
	else if (extentStartVCN >= currentReadVCN && extentStartVCN >= startReadVCN && extentStartVCN < startReadVCN + totalVCNsToRead)
	{
		readMode = true;
		*startReadCluster = extentStartCluster;

		if (startReadVCN + totalVCNsToRead > extentStartVCN + extentSize)
		{
			// ������� �� ����� ��������
			*numberOfClustersToRead = extentSize;
		}
		else
		{
			*numberOfClustersToRead = startReadVCN + totalVCNsToRead - extentStartVCN;
		}
	}

	return readMode;
}
//---------------------------------------------------------------------------
ULONGLONG GetUsedSpaceFromBitmap(BYTE *bitmap, DWORD bitmapSize, DWORD clusterSize)
{
	ULONGLONG usedSpace = 0L;
	BYTE b;
	DWORD i;
	WORD k;

	for (i = 0; i < bitmapSize; i++)
	{
		b = bitmap[i];

		for (k = 0; k < 8; k++)
		{
			if ((b >> k) & 1) usedSpace += clusterSize;
		}
	}

	return usedSpace;
}
//---------------------------------------------------------------------------
//BYTE *AllocateBufferMemory(DataStreamClass *currentStream, DWORD requiredBufferSize, DWORD maxBufferSize, BYTE *currentDataBuffer, DWORD currentDataBufferSize, BYTE **outBufferPointer, DWORD *outBufferSize)
//
//// ������� ������:
//// currentStream - ����� ������, ��� ������ �������� ���������� ������, ��� NULL
//// currentDataBuffer - ������������ ����� ��� NULL, ���� ������ ���
//// currentDataBufferSize - ������ ������������� ������ ��� 0, ���� ������ ���
//
//// �������� ������:
//// tempBufferPointer - ���������, �� �������� ����� ���������� ���������� ������
//// tempBufferSize - ��������� �� ������ ����������� ����� ������, ������� ����� ������������
//
//// ������� ���������� ��������� �� ������, � ������� ����� ����������� ������
//
//// �������������:
//// 1. ������ ������ ������ �� ������� (���� ����� ������ �������)
//// 2. ���� maxBufferSize == 0, �� ������� ������� ��� ������� ������ �� ������
//// 3. ���� requiredBufferSize == 0, �� ���������� �������� � ����� ���� ����� ������
//
//{
//	DWORD minBufferSize = 0;
//	ULONGLONG tempBufferSize = 0;
//	ULONGLONG allocatedDataSize = 0;
//
//	if (currentStream)
//	{
//		minBufferSize = currentStream->GetMinBufferSize();
//		allocatedDataSize = currentStream->GetAllocatedDataSize();
//	}
//	else if (requiredBufferSize)
//	{
//		allocatedDataSize = requiredBufferSize;
//	}
//	else
//	{
//		// ������!!!
//		*outBufferPointer = NULL;
//		*outBufferSize = 0;
//		return NULL;
//	}
//
//	if (requiredBufferSize)
//	{
//		// ���������� ��������� ����� ������ � ������ ������� ������ ������
//		tempBufferSize = (requiredBufferSize > allocatedDataSize ? allocatedDataSize : requiredBufferSize);
//	}
//	else
//	{
//		// �������� �������� ������ ��� ����� ������ ������
//		tempBufferSize = allocatedDataSize;
//	}
//
//	if (maxBufferSize)
//	{
//		// ���������� ��������� ����� ������ � ������ ������������� ������� ������
//		tempBufferSize = (tempBufferSize > maxBufferSize ? maxBufferSize : tempBufferSize);
//	}
//
//	if (minBufferSize)
//	{
//		// ���������, ��� ��������� ����� ������ ������ ������������ ������� ������
//		tempBufferSize = ((tempBufferSize + minBufferSize - 1) / minBufferSize)*minBufferSize;
//	}
//
//	// ���� ��������� ����� �� ��������, ���������� �������� ������
//	if (currentDataBufferSize == 0 || tempBufferSize > currentDataBufferSize)
//	{
//		ClearBuffer(*outBufferPointer);
//		*outBufferPointer = new BYTE[tempBufferSize];
//		*outBufferSize = tempBufferSize;
//		return *outBufferPointer;
//	}
//	else
//	{
//		*outBufferPointer = NULL;
//		*outBufferSize = tempBufferSize;
//		return currentDataBuffer;
//	}
//}
//---------------------------------------------------------------------------
#pragma warn -8070
__int64 __fastcall ReadTSC()
{
	__asm
	{
		rdtsc
	}

	// ��������� ������� ���������� � EAX:EDX, ������� Warning ����� ���������
}
#pragma warn .8070
//---------------------------------------------------------------------------
