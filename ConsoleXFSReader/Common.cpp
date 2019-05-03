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
	default:    return L"не распознана";
	}
}
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
	if (sectorId - startSector >= TotalSectors) return -1; // Будем считать секторы с нуля

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
	if (sectorId - startSector >= TotalSectors) return -1; // Будем считать секторы с нуля

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

	// Задать позицию
	sectorOffset.QuadPart = sectorId * BytesPerSector;

	if (startOffset != NULL)
	{
		*startOffset = sectorOffset.QuadPart;
	}

	startSector = StartOffset / BytesPerSector;

	if (sectorOffset.QuadPart < StartOffset) return 0;
	if (sectorId - startSector >= TotalSectors) return 0; // Будем считать секторы с нуля

	totalBytesToRead = BytesPerSector * numberOfSectors;
	bytesToRead = totalBytesToRead;
	totalBytesRead = 0;

	while (true)
	{
		//leftToRead = 0xFFFFFFFF;
		bytesRead = Storage->ReadDataByOffset(sectorOffset.QuadPart, bytesToRead, &dataBuffer[totalBytesRead], &leftToRead);

		if (bytesRead == 0)
		{
			return 0; // Ошибка
		}

		totalBytesRead += bytesRead;

		if (totalBytesRead == totalBytesToRead) break; // Данные полностью считаны

		// Рассчитать место начала следующего блока, подлежащего считыванию
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

	// Создаем промежуточный буфер, потому что в результате чтения может быть прочитано больше, чем попросили

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

	// Нам нет необходимости возвращать лишние данные
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
	if (clusterId >= TotalClusters) return 0; // Будем считать кластеры с нуля

	// Задать позицию
	LARGE_INTEGER clusterOffset;
	clusterOffset.QuadPart = *startOffset + StartOffset + clusterId * BytesPerCluster;

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
			return totalBytesRead; // Ошибка
		}

		totalBytesRead += bytesRead;

		if (totalBytesRead >= totalBytesToRead) break; // Данные полностью считаны

	// Рассчитать место начала следующего блока, подлежащего считыванию (актуально для образов, разрезанных на части)
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

	// Открыть выходной файл
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
bool FileSystemClass::ClusterIsInBitmapCache(ULONGLONG clusterId)
{
	if (!ClusterBitmapCacheDefined) return false;
	if (clusterId >= ClusterBitmapCacheFirstCluster && clusterId <= ClusterBitmapCacheLastCluster) return true;

	return false;
}
DWORD FileSystemClass::GetNumberOfFiles()
{
	// По умолчанию для файловых систем, не поддерживающих линейный поиск
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

	ULONGLONG totalBytesToRead = (ULONGLONG)VCNsToRead*(ULONGLONG)BytesPerCluster; // Общий объем данных, подлежащих считыванию (без учета размера буфера)
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
		// Буфер слишком маленький
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
}
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
WORD FileObjectClass::GetNumberOfStreams()
{
	return DataStreams.size();
}
//---------------------------------------------------------------------------
void FileObjectClass::AddDataStream(DataStreamClass *newDataStream)
{
	if (newDataStream->GetBytesPerVCN() != 0)
	{
		DataStreams.push_back(newDataStream);
		newDataStream->SetIndex(DataStreams.size() - 1);
	}
}
//---------------------------------------------------------------------------
class DataStreamClass *FileObjectClass::GetFirstDataStream()
{
	if (DataStreams.size() > 0)
	{
		return DataStreams.front();
	}
	else
	{
		return NULL;
	}
}
//---------------------------------------------------------------------------
DataStreamClass::DataStreamClass()
{
	Id = 0;
	Index = 0;

	StreamNameLength = 0;
	StreamName = NULL;

	RealDataSize = 0;
	AllocatedDataSize = 0;

	BytesPerVCN = 0;
}
//---------------------------------------------------------------------------
bool DataStreamClass::IsNormalDataStream() const
{
	return true;
}
//---------------------------------------------------------------------------
DataStreamClass::~DataStreamClass()
{
	if (StreamNameLength > 0) delete[] StreamName;
	StreamName = NULL;
	StreamNameLength = 0;
}
//---------------------------------------------------------------------------
WORD DataStreamClass::GetStreamNameLength() const
{
	return StreamNameLength;
}
//---------------------------------------------------------------------------
const WCHAR *DataStreamClass::GetStreamNamePtr() const
{
	if (StreamNameLength == 0) return NULL;
	else return StreamName;
}
//---------------------------------------------------------------------------
wstring DataStreamClass::GetStreamNameStr() const
{
	if (StreamNameLength == 0) return wstring();
	else return wstring(StreamName, StreamNameLength);
}
//---------------------------------------------------------------------------
WORD DataStreamClass::GetId() const
{
	return Id;
}
//---------------------------------------------------------------------------
void DataStreamClass::SetId(WORD newId)
{
	Id = newId;
}
//---------------------------------------------------------------------------
WORD DataStreamClass::GetIndex() const
{
	return Index;
}
//---------------------------------------------------------------------------
void DataStreamClass::SetIndex(WORD newIndex)
{
	Index = newIndex;
}
//---------------------------------------------------------------------------
ULONGLONG DataStreamClass::GetRealDataSize() const
{
	return RealDataSize;
}
//---------------------------------------------------------------------------
ULONGLONG DataStreamClass::GetAllocatedDataSize() const
{
	return AllocatedDataSize;
}
//---------------------------------------------------------------------------
DWORD DataStreamClass::GetBytesPerVCN() const
{
	return BytesPerVCN;
}
//---------------------------------------------------------------------------
DWORD DataStreamClass::GetMinBufferSize() const
{
	return BytesPerVCN;
}
//---------------------------------------------------------------------------
bool DataStreamClass::SaveDataToFile(WCHAR *fileName, bool forceCreate)
{
	HANDLE fileHandle;
	DWORD minBufferSize, bufferSize;
	BYTE *dataBuffer;
	DWORD readSize;
	ULONGLONG totalReadSize;
	ULONGLONG leftToRead;
	DWORD startVCN;
	DWORD bytesToWrite;
	DWORD bytesWritten;
	ULARGE_INTEGER fileSize;

	// РАБОТАТЬ ЗДЕСЬ!!! Поиск и замена запрещенных в Windows символов.
	// Разобраться, где должен быть этот код!

	// Встречается во вложениях баз данных клиентских почтовых программ (text/plain)
	//ReplaceCharW(fileName, 0, L'/', L'_');

	// Проверить, есть ли файл или каталог с таким именем
	if (!forceCreate)
	{
		DWORD fileAttributes = GetFileAttributesW(fileName);
		if (fileAttributes != INVALID_FILE_ATTRIBUTES)
		{
			if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				return true;
			}
			else
			{
				fileHandle = CreateFileW(
					fileName,
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					//FILE_ATTRIBUTE_NORMAL,
					FILE_FLAG_BACKUP_SEMANTICS,
					NULL
				);

				if (fileHandle != INVALID_HANDLE_VALUE)
				{
					fileSize.QuadPart = 0L;
					fileSize.LowPart = GetFileSize(fileHandle, &fileSize.HighPart);
					CloseHandle(fileHandle);

					if (fileSize.QuadPart == RealDataSize) return true;
				}
			}
		}
	}

	// Создать новый файл
	fileHandle = CreateFileW(
		fileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (fileHandle == INVALID_HANDLE_VALUE) return false;

	if (RealDataSize == 0)
	{
		// Поток данных имеет нулевой размер
		CloseHandle(fileHandle);
		return true;
	}

	// Выделить память для буфера
	minBufferSize = GetMinBufferSize();
	bufferSize = ((ReadBlockSize + minBufferSize - 1) / minBufferSize)*minBufferSize;
	dataBuffer = new BYTE[bufferSize];
	totalReadSize = 0;

	do
	{
		startVCN = totalReadSize / minBufferSize;
		readSize = ReadData(dataBuffer, bufferSize, &leftToRead, startVCN);
		if (readSize == 0 || (leftToRead != 0 && readSize < bufferSize))
		{
			// Ошибка чтения (буфер заполнился не полностью)
			delete[] dataBuffer;
			CloseHandle(fileHandle);
			return false;
		}

		bytesToWrite = (totalReadSize + readSize > RealDataSize ? RealDataSize - totalReadSize : readSize);
		// Записать в файл содержимое буфера
		if (!WriteFile(fileHandle, dataBuffer, bytesToWrite, &bytesWritten, NULL) || (bytesWritten != bytesToWrite))
		{
			// Ошибка записи в файл
			delete[] dataBuffer;
			CloseHandle(fileHandle);
			return false;
		}

		totalReadSize += readSize;

	} while (totalReadSize < RealDataSize);

	// Закрыть файл
	CloseHandle(fileHandle);

	delete[] dataBuffer;
	return true;
}
//---------------------------------------------------------------------------
bool GetExtentReadInfo(
	DWORD startReadVCN,    // Начальный VCN, с которого необходимо начинать чтение
	DWORD totalVCNsToRead, // Общее количество кластеров, которые необходимо считать
	DWORD VCNsToRead,      // Количество кластеров, которые осталось считать
	DWORD currentReadVCN,  // Текущая позиция в файле (очередной VCN)
	DWORD extentStartVCN,  // Начальный VCN экстента
	ULONGLONG extentStartCluster, // Начальный физический кластер экстента
	DWORD extentSize,      // Размер экстента в кластерах
	ULONGLONG *startReadCluster, // Физический кластер, с которого необходимо начать чтение
	DWORD *numberOfClustersToRead // Количество физических кластеров, которые необходимо считать
)
{
	bool readMode = false;

	if (currentReadVCN >= startReadVCN && currentReadVCN >= extentStartVCN && extentStartVCN + extentSize > currentReadVCN)
	{
		// Необходимо начать читать данные в текущем экстенте
		readMode = true;
		*startReadCluster = currentReadVCN - extentStartVCN + extentStartCluster;

		if (currentReadVCN + VCNsToRead > extentStartVCN + extentSize)
		{
			// Считать до конца экстента
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
			// Считать до конца экстента
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
			// Считать до конца экстента
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
BYTE *AllocateBufferMemory(DataStreamClass *currentStream, DWORD requiredBufferSize, DWORD maxBufferSize, BYTE *currentDataBuffer, DWORD currentDataBufferSize, BYTE **outBufferPointer, DWORD *outBufferSize)

// Входные данные:
// currentStream - поток данных, для чтения которого выделяется память, или NULL
// currentDataBuffer - существующий буфер или NULL, если буфера нет
// currentDataBufferSize - размер существующего буфера или 0, если буфера нет

// Выходные данные:
// tempBufferPointer - указатель, по которому можно разместить выделяемую память
// tempBufferSize - указатель на размер выделенного блока данных, который можно использовать

// Функция возвращает указатель на массив, в который можно производить запись

// Предположения:
// 1. Размер потока данных не нулевой (если поток данных передан)
// 2. Если maxBufferSize == 0, то верхней границы для размера буфера не задано
// 3. Если requiredBufferSize == 0, то необходимо уместить в буфер весь поток данных

{
	DWORD minBufferSize = 0;
	ULONGLONG tempBufferSize = 0;
	ULONGLONG allocatedDataSize = 0;

	if (currentStream)
	{
		minBufferSize = currentStream->GetMinBufferSize();
		allocatedDataSize = currentStream->GetAllocatedDataSize();
	}
	else if (requiredBufferSize)
	{
		allocatedDataSize = requiredBufferSize;
	}
	else
	{
		// Ошибка!!!
		*outBufferPointer = NULL;
		*outBufferSize = 0;
		return NULL;
	}

	if (requiredBufferSize)
	{
		// Ограничить требуемый объем памяти с учетом размера потока данных
		tempBufferSize = (requiredBufferSize > allocatedDataSize ? allocatedDataSize : requiredBufferSize);
	}
	else
	{
		// Пытаемся выделить память для всего потока данных
		tempBufferSize = allocatedDataSize;
	}

	if (maxBufferSize)
	{
		// Ограничить требуемый объем памяти с учетом максимального размера буфера
		tempBufferSize = (tempBufferSize > maxBufferSize ? maxBufferSize : tempBufferSize);
	}

	if (minBufferSize)
	{
		// Убедиться, что требуемый объем памяти кратен минимальному размеру буфера
		tempBufferSize = ((tempBufferSize + minBufferSize - 1) / minBufferSize)*minBufferSize;
	}

	// Если имеющийся буфер не подходит, необходимо выделить память
	if (currentDataBufferSize == 0 || tempBufferSize > currentDataBufferSize)
	{
		ClearBuffer(*outBufferPointer);
		*outBufferPointer = new BYTE[tempBufferSize];
		*outBufferSize = tempBufferSize;
		return *outBufferPointer;
	}
	else
	{
		*outBufferPointer = NULL;
		*outBufferSize = tempBufferSize;
		return currentDataBuffer;
	}
}
//---------------------------------------------------------------------------
#pragma warn -8070
__int64 __fastcall ReadTSC()
{
	__asm
	{
		rdtsc
	}

	// Результат функции передается в EAX:EDX, поэтому Warning можно отключить
}
#pragma warn .8070
//---------------------------------------------------------------------------
