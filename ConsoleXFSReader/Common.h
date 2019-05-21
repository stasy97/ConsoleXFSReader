//---------------------------------------------------------------------------
#ifndef CommonH
#define CommonH
//---------------------------------------------------------------------------
#include <wtypes.h>
#include <utility>
#include <stack>
//---------------------------------------------------------------------------
#include "Global.h"
#include "MyStorage.h"
//#include "LibOptionsConst.h"
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
	Iterator<BinaryBlock>* GetClusterIterator(ULONGLONG startCluster = 0, DWORD desiredClustersPerBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);
	virtual class BlockIterator *GetIterator() = 0;
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
//template <class CurrentFileSystemClass, class OutFileObjectClass> 
//OutFileObjectClass * FileSystemClass::ReadFileByFullPathTemplate(const WCHAR *fullPath, size_t fullPathLength)
//{
//	// fullPath = L"\.\Pub\Unclassified\MyFile.txt" (здесь и далее экранирование опущено)
//
//	// Возвращаемые значения:
//	// 1. NTFS: идентификатор файла, запись о файле в каталоге
//	// 2. FAT: запись о файле в каталоге, размер записи
//	// 3. HFS+: запись о файле в каталоге (B-дереве), размер записи, идентификатор родительского каталога
//	// 4. ExtX: идентификатор файла (iNode)
//
//	// В конечном счете должен возвращаться указатель на объект созданного файла,
//	// в конструктор которого передается полный путь, идентификатор файловой записи и файловая запись каталога
//
//	if (fullPathLength == 0) fullPathLength = wcslen(fullPath);
//
//	// Проверить, что путь начинается с корневого каталога
//	if (wcsncmp(fullPath, ROOT_DIR, ROOT_DIR_LENGTH) != 0)
//	{
//		// Неправильный путь (не начинается с корневого каталога)
//		return NULL;
//	}
//
//	// Считать корневой каталог
//	OutFileObjectClass *nextFile = new OutFileObjectClass(static_cast<CurrentFileSystemClass*>(this));
//
//	if (fullPathLength == ROOT_DIR_LENGTH)
//	{
//		// Искомый каталог - корневой
//		return nextFile;
//	}
//
//	// Инициализировать анализируемую строку, отбросив ведущий корневой каталог
//	const WCHAR *tempPath = &fullPath[ROOT_DIR_LENGTH + 1];
//	size_t tempPathLength = fullPathLength - ROOT_DIR_LENGTH - 1;
//	size_t currentPathLength = ROOT_DIR_LENGTH;
//
//	// tempPath = L"Pub\Unclassified\MyFile.txt"
//
//	while (true)
//	{
//		// Выделяем имя каталога (файла), подлежащего считыванию (currentDir)
//		WCHAR *firstDelimiter = wcschr(tempPath, '\\'); // Ищем первый разделитель
//		const WCHAR *nextFileName = tempPath; // currentFile = L"Pub\Unclassified\MyFile.txt"
//		size_t nextFileNameLength;
//
//		if (firstDelimiter != NULL)
//		{
//			// firstDelimiter = L"\Unclassified\MyFile.txt"
//			size_t delimiterPosition = firstDelimiter - tempPath; // Определяем индекс разделителя
//			nextFileNameLength = delimiterPosition;
//
//			// Смещаемся на следующую за разделителем позицию (подготовка к следующей итерации цикла)
//			tempPath = &tempPath[delimiterPosition + 1]; // tempPath = L"Unclassified\MyFile.txt"
//			tempPathLength = tempPathLength - delimiterPosition - 1;
//		}
//		else
//		{
//			// Разделителя нет, потому что в tempPath содержится имя конечного файла
//			nextFileNameLength = tempPathLength;
//			tempPathLength = 0;
//		}
//
//		// Создать итератор для обхода каталога
//		//PDirectoryIterator<wstring> i = nextFile->GetDirectoryNameIterator();
//
//		// Перебрать записи каталога, найти среди них currentFileName
//		bool recordFound = false;
//		for (i->First(); !i->IsDone(); i->Next())
//		{
//			wstring currentFileName = i->GetCurrent();
//
//			// Сравнить полученное и требуемое имена
//			if ((currentFileName.length() == nextFileNameLength) && (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, currentFileName.c_str(), nextFileNameLength, nextFileName, nextFileNameLength) == CSTR_EQUAL))
//			{
//				// Требуемое имя найдено
//				recordFound = true;
//				LONGLONG recordId = i->GetRecordId();
//				BYTE *recordBuffer = NULL;
//				size_t recordSize = i->GetRecordSize();
//
//				if (recordSize > 0)
//				{
//					recordBuffer = new BYTE[recordSize];
//					i->GetRecord(recordBuffer);
//				}
//
//				// Очистить указатель
//				delete nextFile;
//
//				// Создать файловый объект
//				nextFile = new OutFileObjectClass(
//					static_cast<CurrentFileSystemClass*>(this),
//					recordId,
//					currentFileName.c_str(),
//					currentFileName.length(),
//					fullPath, 					// Путь к целевому файлу
//					currentPathLength, 	// Длина пути к целевому файлу
//					recordBuffer,
//					recordSize
//				);
//
//				delete[] recordBuffer;
//
//				if (tempPathLength == 0)
//				{
//					// Найден целевой файл (каталог)
//					return nextFile;
//				}
//				else
//				{
//					currentPathLength += currentFileName.length() + 1;
//					break;
//				}
//			}
//		}
//
//		if (!recordFound) break;
//	}
//
//	delete nextFile;
//	return NULL;
//}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
enum class DirectoryIteratorMode : int
{
	All,
	FileOnly,
	DirOnly,
	Ordered
};
//---------------------------------------------------------------------------
// Каталог-итератор
//---------------------------------------------------------------------------
//template<class ItemType> class DirectoryIterator : public Iterator<ItemType>
//{
//protected:
//
//	BYTE *DirData;
//	DWORD DirDataSize;
//	DWORD DirDataOffset;
//
//	DirectoryIterator()
//	{
//		DirData = NULL;
//		DirDataSize = 0;
//		DirDataOffset = 0;
//	}
//
//public:
//	virtual ~DirectoryIterator()
//	{
//		if (DirData != NULL) { delete[] DirData; DirData = NULL; }
//	}
//
//	virtual void First() { DirDataOffset = 0; }
//	virtual void Next() = 0;
//	virtual bool IsDone() const { return (DirDataOffset >= DirDataSize); }
//	virtual ItemType GetCurrent() const = 0;
//
//	// Специфические функции
//	virtual bool IsDir() const = 0;
//	virtual bool IsLink() const { return false; }
//	virtual bool IsAllocated() const { return true; }
//	virtual bool IsLabel() const { return false; }
//	virtual LONGLONG GetRealDataSize() const = 0;
//	virtual LONGLONG GetRecordId() const = 0;
//	virtual size_t GetRecord(BYTE *recordBuffer = NULL) const = 0;
//	virtual size_t GetRecordSize() const { return GetRecord(); }
//	virtual wstring GetFileName() const = 0;
//	//virtual LONGLONG GetAllocatedDataSize() const = 0;
//};
//---------------------------------------------------------------------------
// Специальный класс для гарантированного удаления каталог-итераторов
// (точная копия класса PIterator)
//---------------------------------------------------------------------------
//template<class Type>
//class PDirectoryIterator
//{
//public:
//	PDirectoryIterator(DirectoryIterator<Type> * it) { It = it; }
//	virtual ~PDirectoryIterator() { if (It != NULL) delete It; }
//	DirectoryIterator<Type>* operator->() { return It; }
//	bool IsNull() { return (It == NULL); }
//
//private:
//	DirectoryIterator<Type> * It;
//
//	// Запретить копирование и присваивание,
//	// чтобы избежать многократных удалений
//	PDirectoryIterator(const PDirectoryIterator&);
//	PDirectoryIterator& operator=(const PDirectoryIterator&);
//
//	// Запретить разыменование, чтобы не нужно было обрабатывать ситуацию, когда It == NULL
//	PDirectoryIterator& operator*();
//	//PIterator& operator*() { return *It; }
//};
//---------------------------------------------------------------------------
// Декоратор каталога-итератора для перебора только существующих файлов
//---------------------------------------------------------------------------
// РАБОТАТЬ ЗДЕСЬ!!!
//---------------------------------------------------------------------------
// Декоратор каталога-итератора для перебора только файлов
//---------------------------------------------------------------------------
//template<class ItemType> class DirectoryFileOnlyIteratorDecorator : public DirectoryIterator<ItemType>
//{
//protected:
//	DirectoryIterator<ItemType> *It;
//
//public:
//	DirectoryFileOnlyIteratorDecorator(DirectoryIterator<ItemType> *it) { It = it; First(); }
//	virtual ~DirectoryFileOnlyIteratorDecorator() { delete It; }
//	virtual void First() { It->First();	while (!It->IsDone() && It->IsDir()) { It->Next(); } }
//	virtual void Next() { do { It->Next(); } while (!It->IsDone() && It->IsDir()); }
//	virtual bool IsDone() const { return It->IsDone(); }
//	virtual ItemType GetCurrent() const { return It->GetCurrent(); }
//	virtual bool IsDir() const { return It->IsDir(); }
//	virtual bool IsLink() const { return It->IsLink(); }
//	virtual bool IsAllocated() const { return It->IsAllocated(); }
//	virtual bool IsLabel() const { return It->IsLabel(); }
//	virtual LONGLONG GetRealDataSize() const { return It->GetRealDataSize(); }
//	virtual LONGLONG GetRecordId() const { return It->GetRecordId(); }
//	virtual size_t GetRecordSize() const { return It->GetRecordSize(); }
//	virtual size_t GetRecord(BYTE *recordBuffer) const { return It->GetRecord(recordBuffer); }
//	virtual wstring GetFileName() const { return It->GetFileName(); }
//};
//---------------------------------------------------------------------------
// Декоратор каталога-итератора для перебора только каталогов
//---------------------------------------------------------------------------
//template<class ItemType> class DirectoryDirOnlyIteratorDecorator : public DirectoryIterator<ItemType>
//{
//protected:
//	DirectoryIterator<ItemType> *It;
//
//public:
//	DirectoryDirOnlyIteratorDecorator(DirectoryIterator<ItemType> *it) { It = it; First(); }
//	virtual ~DirectoryDirOnlyIteratorDecorator() { delete It; }
//	virtual void First() { It->First();	while (!It->IsDone() && !It->IsDir()) { It->Next(); } }
//	virtual void Next() { do { It->Next(); } while (!It->IsDone() && !It->IsDir()); }
//	virtual bool IsDone() const { return It->IsDone(); }
//	virtual ItemType GetCurrent() const { return It->GetCurrent(); }
//	virtual bool IsDir() const { return It->IsDir(); }
//	virtual bool IsLink() const { return It->IsLink(); }
//	virtual bool IsAllocated() const { return It->IsAllocated(); }
//	virtual bool IsLabel() const { return It->IsLabel(); }
//	virtual LONGLONG GetRealDataSize() const { return It->GetRealDataSize(); }
//	virtual LONGLONG GetRecordId() const { return It->GetRecordId(); }
//	virtual size_t GetRecordSize() const { return It->GetRecordSize(); }
//	virtual size_t GetRecord(BYTE *recordBuffer) const { return It->GetRecord(recordBuffer); }
//	virtual wstring GetFileName() const { return It->GetFileName(); }
//};
//---------------------------------------------------------------------------
// Декоратор каталога-итератора для перебора сначала каталогов, потом файлов
//---------------------------------------------------------------------------
//template<class ItemType> class DirectoryOrderedIteratorDecorator : public DirectoryIterator<ItemType>
//{
//protected:
//	DirectoryIterator<ItemType> *It;
//	DirectoryIteratorMode Mode;
//
//public:
//	DirectoryOrderedIteratorDecorator(DirectoryIterator<ItemType> *it)
//	{
//		It = it;
//		First();
//	}
//
//	virtual ~DirectoryOrderedIteratorDecorator() { delete It; }
//
//	virtual void First()
//	{
//		Mode = DirectoryIteratorMode::DirOnly;
//		It->First();
//		while (!It->IsDone() && !It->IsDir()) { Next(); }
//		if (It->IsDone())
//		{
//			Mode = DirectoryIteratorMode::FileOnly;
//			It->First();
//			while (!It->IsDone() && It->IsDir()) { Next(); }
//		}
//	}
//
//	virtual void Next()
//	{
//		if (Mode == DirectoryIteratorMode::DirOnly)
//		{
//			do { It->Next(); } while (!It->IsDone() && !It->IsDir());
//			if (It->IsDone()) { Mode = DirectoryIteratorMode::FileOnly; It->First(); while (!It->IsDone() && It->IsDir()) { Next(); } }
//		}
//		else if (Mode == DirectoryIteratorMode::FileOnly)
//		{
//			do { It->Next(); } while (!It->IsDone() && It->IsDir());
//		}
//	}
//
//	virtual bool IsDone() const { return ((Mode == DirectoryIteratorMode::FileOnly) && It->IsDone()); }
//	virtual ItemType GetCurrent() const { return It->GetCurrent(); }
//	virtual bool IsDir() const { return It->IsDir(); }
//	virtual bool IsLink() const { return It->IsLink(); }
//	virtual bool IsAllocated() const { return It->IsAllocated(); }
//	virtual bool IsLabel() const { return It->IsLabel(); }
//	virtual LONGLONG GetRealDataSize() const { return It->GetRealDataSize(); }
//	virtual LONGLONG GetRecordId() const { return It->GetRecordId(); }
//	virtual size_t GetRecordSize() const { return It->GetRecordSize(); }
//	virtual size_t GetRecord(BYTE *recordBuffer) const { return It->GetRecord(recordBuffer); }
//	virtual wstring GetFileName() const { return It->GetFileName(); }
//};
//---------------------------------------------------------------------------
// Файловый объект
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
	//list<class DataStreamClass*> DataStreams;

	// Операции с потоками данных
	//void AddDataStream(class DataStreamClass *newDataStream);
	//void ClearDataStreams();

	// Контекст файлового объекта (цепочка файлов и файловых систем)
	std::stack<PFileContext> Context;

public:
	FileObjectClass();
	//virtual ~FileObjectClass();

	//virtual class FileSystemClass *GetFileSystem() = 0;
	void SetContext(std::stack<PFileContext>& newContext) { Context = newContext; }

	virtual bool IsDir() = 0;
	virtual bool IsLink() { return false; }
	virtual bool IsUsed() { return true; }
	//virtual bool IsEncrypted() { return false; }
	//virtual bool IsRoot();
	//virtual LONGLONG GetRecordId();
	//virtual LONGLONG GetBitmapId();
	//virtual LONGLONG GetParentId();
	//virtual LONGLONG GetBaseId(); // Для NTFS - идентификатор базовой записи, для FAT - номер первого кластера данных
	//virtual bool IsMarkedInBitmap(const BYTE *fileBitmap);
	//virtual void MarkFileInBitmap(BYTE *fileBitmap);
	//virtual WORD GetCommonFlags();
	//virtual DWORD GetStdInfoFlags();
	virtual wstring PrintFileFlags() { return L""; }

	// Операции с именем файлового объекта
	//virtual WORD GetFileNameLength();
	//virtual WORD GetFilePathLength();
	//virtual WORD GetFullPathLength();
	//virtual WORD GetFileName(WCHAR *targetBuffer = NULL);
	//virtual WORD GetFilePath(WCHAR *targetBuffer = NULL);
	//virtual WORD GetFullPath(WCHAR *targetBuffer = NULL);
	//const WCHAR *GetFileNamePtr();
	//const WCHAR *GetFilePathPtr();
	//wstring GetFileNameStr();
	//wstring GetFilePathStr();
	//wstring GetFullPathStr();
	void ClearFileName();
	void ClearFilePath();
	//bool SameName(const WCHAR *fileName);
	//bool SameFiles(const WCHAR *fullPath);
	bool SameFiles(FileObjectClass *fileObject);
	bool SamePath(const WCHAR *fullPath);
	//bool Contains(const WCHAR *fullPath);

	// Операции с файловой записью
	virtual DWORD GetFileRecordSize();
	virtual DWORD GetFileRecordRealSize() = 0;
	virtual WORD GetFileRecordType(CHAR* fileType) = 0;
	virtual DWORD GetFileRecord(BYTE *dataBuffer = NULL);
	virtual LONGLONG GetRecordOffset();
	//virtual DirectoryIterator<wstring>* GetDirectoryNameIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All) = 0;
	//virtual DirectoryIterator<FileObjectClass*>* GetDirectoryFileIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All) = 0;
	//virtual DirectoryIterator<DirectoryEntryRecord>* GetDirectoryRecordIterator(DirectoryIteratorMode mode = DirectoryIteratorMode::All) = 0;

	// Операции с временными отметками
	virtual bool ReadTimeStamps() = 0;
	LARGE_INTEGER GetTimeFileCreate();
	LARGE_INTEGER GetTimeFileModify();
	LARGE_INTEGER GetTimeFileAccess();
	LARGE_INTEGER GetTimeRecordModify();

	// Операции с потоками данных
	//virtual WORD GetNumberOfStreams();
	//virtual Iterator<class DataStreamClass*>* GetDataStreamIterator();
	/*virtual class DataStreamClass *GetFirstDataStream();
	virtual class DataStreamClass *GetDataStreamById(WORD streamId) = 0;
	virtual class DataStreamClass *GetDataStreamByIndex(WORD streamIndex) = 0;*/
	list< pair<int, wstring> > __fastcall GetFileStreamNames();

	bool SaveTo(WCHAR *targetFolder, const WCHAR *fileName, bool absolutePath, bool includeSubdirs = true, bool copyFileTime = true, bool addRecordId = false);
};
//---------------------------------------------------------------------------


typedef FileObjectClass * PFileObjectClass;
//---------------------------------------------------------------------------
// Поток данных файла
//---------------------------------------------------------------------------
//class DataStreamClass
//{
//protected:
//
//	//FileObjectClass *ParentFileObject;
//	WORD Id;
//	WORD Index;
//
//	WCHAR *StreamName;
//	WORD StreamNameLength;
//
//	ULONGLONG RealDataSize;
//	ULONGLONG AllocatedDataSize;
//	DWORD     BytesPerVCN;
//
//public:
//
//	DataStreamClass();
//	virtual ~DataStreamClass();
//
//	virtual bool IsCompressed() const { return false; }
//	virtual bool IsNormalDataStream() const; // Для обычный потоков данных TRUE, для виртуальных (типа файловых ссылок) - FALSE.
//
//	virtual WORD GetId() const;
//	virtual void SetId(WORD newId);
//	virtual WORD GetIndex() const;
//	virtual void SetIndex(WORD newIndex);
//
//	virtual WORD GetStreamNameLength() const;
//	//virtual WORD GetStreamName(WCHAR *targetBuffer = NULL) const;
//	const WCHAR *GetStreamNamePtr() const;
//	wstring GetStreamNameStr() const;
//
//	virtual DWORD GetMinBufferSize() const;
//	virtual ULONGLONG GetRealDataSize() const;
//	virtual ULONGLONG GetAllocatedDataSize() const;
//	DWORD GetBytesPerVCN() const;
//
//	virtual DWORD ReadData(BYTE *buffer, DWORD bufferSize, ULONGLONG *leftToRead, DWORD startReadVCN = 0, DWORD VCNsToRead = 0, bool checkSize = true) = 0;
//	Iterator<BinaryBlock>* GetIterator(ULONGLONG startVCN = 0, DWORD desiredVCNsPerBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);
//
//	//virtual DWORD ReadDataSlack(BYTE *dataBuffer) = 0; // Чтение "хвоста" потока данных
//	virtual bool GetSectorByOffset(ULONGLONG inOffset, LONGLONG *outSector, WORD *outOffset) { return false; }
//	bool SaveDataToFile(WCHAR *fileName, bool forceCreate = false);
//	virtual void MarkClusterBitmap(BYTE *clusterBitmap, DWORD clusterBitmapSize) {};
//	virtual DWORD GetNumberOfOverwrittenClusters() { return 0; }
//};
//---------------------------------------------------------------------------
class BinaryBlockIterator : public Iterator<BinaryBlock>
{
protected:

	ULONGLONG StartIndex;
	ULONGLONG CurrentIndex;
	DWORD BytesPerCluster;
	ULONGLONG NumberOfClusters;
	DWORD ClustersPerReadBlock;
	DWORD StepSizeInClusters;
	DWORD ReadBlockSizeInBytes;

public:

	BinaryBlockIterator(
		ULONGLONG startIndex,
		DWORD bytesPerCluster,
		ULONGLONG numberOfClusters,
		DWORD minReadBlock,
		DWORD desiredClustersPerReadBlock,
		bool needOverlap,
		DWORD *outOverlapSize
	);

	virtual void First() { CurrentIndex = StartIndex; }
	virtual void Next() { CurrentIndex += StepSizeInClusters; }
	virtual bool IsDone() const { return (CurrentIndex >= NumberOfClusters); }
	virtual BinaryBlock GetCurrent() const = 0;
};
//---------------------------------------------------------------------------
class FileSystemClusterIterator : public BinaryBlockIterator
{
protected:

	FileSystemClass *FileSystem;

public:
	FileSystemClusterIterator(FileSystemClass *fileSystem, ULONGLONG startCluster = 0, DWORD desiredClustersPerReadBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);
	virtual BinaryBlock GetCurrent() const;
};
//---------------------------------------------------------------------------
//class DataStreamVCNIterator : public BinaryBlockIterator
//{
//protected:
//
//	DataStreamClass *DataStream;
//
//public:
//	DataStreamVCNIterator::DataStreamVCNIterator(DataStreamClass *dataStream, ULONGLONG startVCN = 0, DWORD desiredVCNsPerBlock = 0, bool needOverlap = false, DWORD *outOverlapSize = NULL);
//	virtual BinaryBlock GetCurrent() const;
//};
//---------------------------------------------------------------------------
//bool __fastcall CacheImageStream(DataStreamClass *dataStream, ULONGLONG recordId, WORD streamNumber, const WCHAR *outCacheDir);
//---------------------------------------------------------------------------
//bool SaveDir(
//	PIterator<FileObjectClass*>& dirIterator,
//	WCHAR *targetFolder,
//	bool recoverPath,
//	bool includeSubdirs,
//	bool copyFileTime = true
//);
//---------------------------------------------------------------------------
//bool SaveDirAsText(
//	WCHAR *fullDirPath,
//	PIterator<FileObjectClass*>& dirIterator,
//	WCHAR *fileName,
//	bool includeSubdirs,
//	bool unicode = false,
//	TIME_ZONE_INFORMATION *timeZoneInformation = NULL,
//	HANDLE fileHandle = 0
//);
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
//BYTE *AllocateBufferMemory(DataStreamClass *currentStream, DWORD requiredBufferSize, DWORD maxBufferSize, BYTE *currentDataBuffer, DWORD currentDataBufferSize, BYTE **tempBufferPointer, DWORD *tempBufferSize);
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
//FileSystemTypeEnum RecognizeSfxArchiveType(DataStreamClass *dataStream);
//---------------------------------------------------------------------------
__int64 __fastcall ReadTSC();
//---------------------------------------------------------------------------
#endif
