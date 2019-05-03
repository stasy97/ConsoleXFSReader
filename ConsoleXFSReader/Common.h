//---------------------------------------------------------------------------
#ifndef CommonH
#define CommonH
//---------------------------------------------------------------------------
#include <wtypes.h>
#include <utility>
#include <stack>
//---------------------------------------------------------------------------
#include "MyStorage.h"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
const int FILENAME_MAX_LENGTH = 256;
const DWORD DefaultClusterSize = 4096;
const DWORD ReadBlockSize = 0x800000; // Максимальный размер считываемого блока данных
const DWORD MaxArchiveHeaderSize = 0x10000;
const DWORD MaxSignatureSize = 0x1000;
const DWORD MaxBitmapCacheSize = ReadBlockSize;
const WCHAR ROOT_DIR[] = L"\\.";
const int ROOT_DIR_LENGTH = 2;
//---------------------------------------------------------------------------
typedef struct
{
	LONGLONG RecordId;
	DWORD BitmapId;
	LONGLONG FileSize;
	LARGE_INTEGER TimeFileCreate;
	LARGE_INTEGER TimeFileModify;
	LARGE_INTEGER TimeFileAccess;
	LARGE_INTEGER TimeRecordModify;
	DWORD FileFlags;
	bool IsParent;
	bool IsDir;
	bool IsLink;
	bool IsLabel;

	wstring Name;
	wstring FileFlagsString;

} DirectoryEntryRecord, *PDirectoryEntryRecord;
//---------------------------------------------------------------------------
typedef struct
{
	DirectoryEntryRecord BasicData;

	struct
	{
		wstring TimeFileCreateString;
		wstring TimeFileModifyString;
		wstring TimeFileAccessString;
		wstring TimeRecordModifyString;

	} TimeStampStrings;

	struct
	{
		bool IsKnown;
		bool IsFound;
		bool IsInspected;

	} BitmapData;

	struct
	{
		bool IsContainer;
		bool IsImage;
		bool IsWindowsLink;

	} DecodedData;

} DirectoryEntryTreeRecord, *PDirectoryEntryTreeRecord;
//---------------------------------------------------------------------------
// Абстрактная файловая система (раздел диска)
//---------------------------------------------------------------------------
class FileSystemClass
{
protected:
	bool Error;

	FileSystemTypeEnum Type;
	BYTE Serial[17];
	WORD SerialLength;

	WORD BytesPerSector;
	WORD SectorsPerCluster;

	// Вычисляется и хранится, чтобы каждый раз считать не надо было
	DWORD BytesPerCluster;

	ULONGLONG TotalSectors;
	ULONGLONG TotalClusters;

	// Должны задаваться на этапе инициализаци
	ULONGLONG StartOffset; // Начало раздела (смещение в файле)
	StorageClass *Storage; // Указатель на носитель (виртуальный файл)
	bool StorageAllocated;

	bool ClusterBitmapCacheDefined;
	BYTE *ClusterBitmapCache;
	DWORD ClusterBitmapSize;
	ULONGLONG ClusterBitmapCacheFirstCluster;
	ULONGLONG ClusterBitmapCacheLastCluster;
	bool ClusterIsInBitmapCache(ULONGLONG clusterId);
	bool ReadClusterStateFromCache(ULONGLONG clusterId);
	//virtual bool ReadClusterStateFromDisk(ULONGLONG clusterId) = 0;

	template <class CurrentFileSystemClass, class OutFileObjectClass> OutFileObjectClass * ReadFileByFullPathTemplate(const WCHAR *fullPath, size_t fullPathLength = 0);

	virtual void Init(StorageClass *dataStorage, ULONGLONG startOffset, ULONGLONG diskSize = 0, WORD sectorSize = 0, DWORD clusterSize = 0) = 0;
	FileSystemClass();
	FileSystemClass(FileSystemClass *srcFileSystem);

public:

	// Конструктор файловой системы методом копирования
	//virtual FileSystemClass *Copy() = 0;

	// Деструктор
	virtual ~FileSystemClass();
	
	//Информация о файловой системе
	virtual void ShowInfo() = 0;

	virtual bool GetError();
	void SetError();
	void ClearError();

	FileSystemTypeEnum GetType();
	string GetTypeString();
	wstring GetTypeStringW();
	const BYTE * GetSerialPtr();
	string GetSerialString();
	wstring GetSerialStringW();
	ULONGLONG GetStartOffset();
	WORD GetBytesPerSector() const;
	WORD GetSectorsPerCluster();
	DWORD GetBytesPerCluster();

	ULONGLONG GetTotalClusters();
	ULONGLONG GetTotalSectors();

	LONGLONG GetSectorIdByOffset(ULONGLONG dataOffset) const;
	LONGLONG GetOffsetBySectorId(ULONGLONG sectorId) const;
	LONGLONG GetOffsetByClusterId(ULONGLONG clusterId) const;
	//virtual LONGLONG GetOffsetByRecordId(ULONGLONG recordId) = 0;
	virtual LONGLONG GetClusterIdBySector(ULONGLONG sectorId) const;
	virtual LONGLONG GetSectorIdByCluster(ULONGLONG clusterId) const;

	// Чтение произвольных блоков данных
	DWORD ReadDataByOffset(ULONGLONG startOffset, DWORD bytesToRead, BYTE *dataBuffer, DWORD *leftToRead = NULL);

	// Чтение секторов
	DWORD ReadSectorsByNumber(ULONGLONG sectorId, DWORD numberOfSectors, BYTE *dataBuffer, ULONGLONG *startOffset = NULL);

	// Проверка диапазона читаемых данных
	bool CheckReadDataRange(const ULONGLONG realDataSize, const ULONGLONG allocatedDataSize, const DWORD bufferSize, const DWORD startReadVCN, const DWORD VCNsToRead, ULONGLONG *outTotalBytesToRead, DWORD *outVCNsToRead, bool extendedRead = false);

	// Чтение кластеров
	virtual DWORD ReadClustersByNumber(ULONGLONG clusterId, DWORD numberOfClusters, BYTE *dataBuffer, ULONGLONG *startOffset = NULL) const;
	bool ClusterIsAllocated(ULONGLONG clusterId);
	//virtual bool ClusterBitmapCacheInit(ULONGLONG clusterId) = 0;
	bool SaveClustersToFolder(ULONGLONG clusterId, DWORD numberOfClusters, WCHAR *targetFolder);
	bool SaveClustersToFile(ULONGLONG clusterId, DWORD numberOfClusters, WCHAR *targetFile);
	//Iterator<BinaryBlock>* GetClusterIterator(ULONGLONG startCluster = 0, DWORD desiredClustersPerBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);

	// Работа с файлами
	//virtual DWORD GetFileRecordSize() = 0;
	virtual DWORD GetNumberOfFiles();
	//virtual class FileObjectClass *ReadFileByFullPath(const WCHAR *fullPath) = 0;
	//virtual class FileObjectClass *ReadFileByRecordId(ULONGLONG recordId) = 0;
	//virtual DWORD GetFileRecordBitmapSize() = 0;
	//virtual ULONGLONG GetUsedSpace() = 0;
	//virtual WORD GetVolumeLabel(WCHAR *targetBuffer = NULL) = 0;
};
//---------------------------------------------------------------------------
typedef FileSystemClass * PFileSystemClass;
//---------------------------------------------------------------------------
enum class DirectoryIteratorMode : int
{
	All,
	FileOnly,
	DirOnly,
	Ordered
};
//---------------------------------------------------------------------------
class FileObjectClass;

typedef std::pair<FileObjectClass*, FileSystemClass*> PFileContext;
//---------------------------------------------------------------------------
class FileObjectClass
{
protected:
	bool RecordIdDefined;
	LONGLONG RecordId;
	bool RootDir;

	bool FileObjectIsUnpacked;

	bool RecordBufferDefined;
	BYTE *RecordBuffer;
	ULONGLONG RecordOffset;
	DWORD FileRecordSize;

	bool FilePathDefined;
	WCHAR *FilePath;
	WORD FilePathLength;

	bool FileNameDefined;
	WCHAR *FileName;
	WORD FileNameLength;

	bool TimeStampsDefined;
	LARGE_INTEGER TimeFileCreate;
	LARGE_INTEGER TimeFileModify;
	LARGE_INTEGER TimeFileAccess;
	LARGE_INTEGER TimeRecordModify;

	bool DataStreamsDefined;
	list<class DataStreamClass*> DataStreams;

	// Операции с потоками данных
	void AddDataStream(class DataStreamClass *newDataStream);
	void ClearDataStreams();

	// Контекст файлового объекта (цепочка файлов и файловых систем)
	std::stack<PFileContext> Context;

public:
	FileObjectClass();
	//virtual ~FileObjectClass();

	//virtual class FileSystemClass *GetFileSystem() = 0;
	void SetContext(std::stack<PFileContext>& newContext) { Context = newContext; }

	// Операции с именем файлового объекта
	void ClearFileName();
	void ClearFilePath();
	bool SameFiles(FileObjectClass *fileObject);
	bool SamePath(const WCHAR *fullPath);
	//bool Contains(const WCHAR *fullPath);

	// Операции с файловой записью
	virtual DWORD GetFileRecordSize();
	virtual DWORD GetFileRecordRealSize() = 0;
	virtual WORD GetFileRecordType(CHAR* fileType) = 0;
	virtual DWORD GetFileRecord(BYTE *dataBuffer = NULL);
	virtual LONGLONG GetRecordOffset();

	// Операции с временными отметками
	virtual bool ReadTimeStamps() = 0;
	LARGE_INTEGER GetTimeFileCreate();
	LARGE_INTEGER GetTimeFileModify();
	LARGE_INTEGER GetTimeFileAccess();
	LARGE_INTEGER GetTimeRecordModify();

	// Операции с потоками данных
	virtual WORD GetNumberOfStreams();
	//virtual Iterator<class DataStreamClass*>* GetDataStreamIterator();
	virtual class DataStreamClass *GetFirstDataStream();
	virtual class DataStreamClass *GetDataStreamById(WORD streamId) = 0;
	virtual class DataStreamClass *GetDataStreamByIndex(WORD streamIndex) = 0;
	list< pair<int, wstring> > __fastcall GetFileStreamNames();

	bool SaveTo(WCHAR *targetFolder, const WCHAR *fileName, bool absolutePath, bool includeSubdirs = true, bool copyFileTime = true, bool addRecordId = false);
};
//---------------------------------------------------------------------------


typedef FileObjectClass * PFileObjectClass;
//---------------------------------------------------------------------------
// Поток данных файла
//---------------------------------------------------------------------------
class DataStreamClass
{
protected:

	//FileObjectClass *ParentFileObject;
	WORD Id;
	WORD Index;

	WCHAR *StreamName;
	WORD StreamNameLength;

	ULONGLONG RealDataSize;
	ULONGLONG AllocatedDataSize;
	DWORD     BytesPerVCN;

public:

	DataStreamClass();
	virtual ~DataStreamClass();

	virtual bool IsCompressed() const { return false; }
	virtual bool IsNormalDataStream() const; // Для обычный потоков данных TRUE, для виртуальных (типа файловых ссылок) - FALSE.

	virtual WORD GetId() const;
	virtual void SetId(WORD newId);
	virtual WORD GetIndex() const;
	virtual void SetIndex(WORD newIndex);

	virtual WORD GetStreamNameLength() const;
	//virtual WORD GetStreamName(WCHAR *targetBuffer = NULL) const;
	const WCHAR *GetStreamNamePtr() const;
	wstring GetStreamNameStr() const;

	virtual DWORD GetMinBufferSize() const;
	virtual ULONGLONG GetRealDataSize() const;
	virtual ULONGLONG GetAllocatedDataSize() const;
	DWORD GetBytesPerVCN() const;

	virtual DWORD ReadData(BYTE *buffer, DWORD bufferSize, ULONGLONG *leftToRead, DWORD startReadVCN = 0, DWORD VCNsToRead = 0, bool checkSize = true) = 0;
	//Iterator<BinaryBlock>* GetIterator(ULONGLONG startVCN = 0, DWORD desiredVCNsPerBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);

	//virtual DWORD ReadDataSlack(BYTE *dataBuffer) = 0; // Чтение "хвоста" потока данных
	virtual bool GetSectorByOffset(ULONGLONG inOffset, LONGLONG *outSector, WORD *outOffset) { return false; }
	bool SaveDataToFile(WCHAR *fileName, bool forceCreate = false);
	virtual void MarkClusterBitmap(BYTE *clusterBitmap, DWORD clusterBitmapSize) {};
	virtual DWORD GetNumberOfOverwrittenClusters() { return 0; }
};
//---------------------------------------------------------------------------
bool __fastcall CacheImageStream(DataStreamClass *dataStream, ULONGLONG recordId, WORD streamNumber, const WCHAR *outCacheDir);
//---------------------------------------------------------------------------
// Запись каталога
//---------------------------------------------------------------------------
class DirectoryEntryClass
{
protected:
	BYTE *DirData;
	DWORD DirDataSize;
	DWORD DirDataOffset;
	DWORD Size;

	bool FileNameDefined;
	bool FileNameAllocated;
	WCHAR *FileName;
	WORD FileNameLength;
	bool Allocated;

	WCHAR *DirPath;      // Указатель на переменную DirPath каталога
	DWORD DirPathLength;

public:
	DirectoryEntryClass();
	virtual ~DirectoryEntryClass();

	virtual bool IsDir() = 0;
	virtual bool IsLink();
	virtual bool IsAllocated();
	virtual bool IsLast() = 0;
	virtual bool IsLabel();
	virtual class DirectoryEntryClass *First() = 0;
	virtual class DirectoryEntryClass *Next() = 0; // Возвращается адрес новой записи или NULL в случае ошибки
	virtual ULONGLONG GetRecordId() = 0;
	virtual DWORD GetFirstClusterId(); // Номер первого кластера в файловой системе FAT
	virtual class FileObjectClass *AsFileObject() = 0;

	// Операции с именем записи каталога
	virtual WORD GetFileNameLength();
	virtual WORD GetFileName(WCHAR *targetBuffer = NULL);

	virtual LONGLONG GetFileSize() = 0;

	// Операции с временными отметками
	virtual LARGE_INTEGER GetTimeFileCreate() = 0;
	virtual LARGE_INTEGER GetTimeFileModify() = 0;
	virtual LARGE_INTEGER GetTimeFileAccess() = 0;
	virtual LARGE_INTEGER GetTimeRecordModify() = 0;

	// Чтение флагов в виде строки, которая затем отображается на экране,
	// а также в виде 4-байтового числа, специфичного для каждой ФС
	virtual DWORD GetFileFlags(CHAR *flagsBuffer = NULL) = 0;

	// Чтение файловой записи
	virtual DWORD GetData(BYTE *buffer);
};
//---------------------------------------------------------------------------
BYTE *AllocateBufferMemory(DataStreamClass *currentStream, DWORD requiredBufferSize, DWORD maxBufferSize, BYTE *currentDataBuffer, DWORD currentDataBufferSize, BYTE **tempBufferPointer, DWORD *tempBufferSize);
inline void _fastcall ClearBuffer(BYTE *dataBuffer) { if (dataBuffer != NULL) { delete[] dataBuffer; dataBuffer = NULL; } }
//---------------------------------------------------------------------------
bool ForceDirectory(const WCHAR *dirPath);
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
);
//---------------------------------------------------------------------------
ULONGLONG GetUsedSpaceFromBitmap(BYTE *bitmap, DWORD bitmapSize, DWORD clusterSize);
//---------------------------------------------------------------------------
#pragma pack(push, 1)
//---------------------------------------------------------------------------
typedef union
{
	struct
	{
		WORD TimeFileHMS;
		WORD TimeFileD;

	} AsStruct;

	DWORD AsInt32;

} DosTimeStruct, *PDosTimeStruct;
//---------------------------------------------------------------------------
#pragma pack(pop)
//---------------------------------------------------------------------------
wstring GetFileSystemTypeStringW(FileSystemTypeEnum fsType);
wstring __fastcall ConvertFileTime(LARGE_INTEGER time, TIME_ZONE_INFORMATION *timeZoneInformation = NULL);
//---------------------------------------------------------------------------
FileSystemTypeEnum RecognizeSfxArchiveType(DataStreamClass *dataStream);
//---------------------------------------------------------------------------
__int64 __fastcall ReadTSC();
//---------------------------------------------------------------------------
#endif
