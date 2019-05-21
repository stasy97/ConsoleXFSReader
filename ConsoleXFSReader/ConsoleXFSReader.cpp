// XFS_lib.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <wtypes.h>

#include "MyStorage.h"
#include "Common.h"
#include "LibFileSystem.h"
#include "FileSystemExportConst.h"
#include "OtherFunc.h"
#include "XFS.h"

//WCHAR *fileName1 = L"\\\\.\\PhysicalDrive0"; // \\.\PhysicalDrive0, \\.\PhysicalDrive1 и т. д.
//WCHAR *fileName = L"\\\\.\\G:"; // \\.\C:, \\.\D: и т. д.
// Либо просто имя файла


int main()
{
	setlocale(LC_ALL, "Russian");
	bool noError = true;

	//StorageClass *dataStorage = new SimpleStorageClass(L"\\\\.\\F:"); - флешка

	StorageClass *dataStorage = NULL;
	dataStorage = new SimpleStorageClass(L"\\\\.\\D:\\CentOS\\CentOS 64-bit-flat.vmdk"); //виртуальный диск

	noError = dataStorage->Open();

	if (!noError)
	{
		dataStorage->Close();
		cout << "Ошибка открытия диска/файла.\nВыполнение программы завершено!\n";
		system("PAUSE");
		return -1;
	}

	StorageType t = dataStorage->GetType();
	ULONGLONG size = dataStorage->GetDataSize();

	cout << "Открыт носитель типа ";
	if (static_cast<underlying_type<StorageType>::type>(t) == 0) cout << "LogicalDrive. ";
	else if (static_cast<underlying_type<StorageType>::type>(t) == 1) cout << "ImageFile. ";
	cout << "Размер носителя - " << size << " байт." << endl;

	FileSystemClass *fileSystem = CreateFileSystem(FileSystemTypeEnum::XFS, dataStorage);
	if (fileSystem->GetError())
	{
		cout << "Ошибка с файловой системой! \nВыполнение программы завершено!\n" << endl;
		cin.get();
		system("PAUSE");
		return -1;
	}
	cout << endl;
	cout << "Информация о файловой системе: " << endl;
	fileSystem->ShowInfo();

	//cout << "Введите номер блока, который необходимо считать. Введите 0, чтобы пропустить считывание." << endl;
	//ULONGLONG clusterid = 0;
	//cin >> clusterid;
	//while (clusterid < 0 || clusterid >= fileSystem->GetTotalClusters()) {
	//	cout << "Номер блока не должен быть меньше 0 и больше " << dec << fileSystem->GetTotalClusters() << endl;
	//	cin >> clusterid;
	//}

	//if (clusterid) {
	//	BYTE * buffer = new BYTE[fileSystem->GetBytesPerCluster()];
	//	ULONGLONG startoffset = 0;
	//	DWORD result = fileSystem->ReadClustersByNumber(clusterid - 1, 1, buffer, &startoffset);
	//	ShowHexData(buffer, fileSystem->GetBytesPerCluster());
	//}


	//Итератор по блокам
	BlockIterator *it = fileSystem->GetIterator();
	
	cout << "\t Iterate SuperBlocks."<< endl;
	//Итератор по супер-блокам (декоратор)
	SB_IteratorDecorator * SB_iterator = new SB_IteratorDecorator(it);
	for (SB_iterator->First(); !SB_iterator->IsDone(); SB_iterator->Next())
	{		
	}

	dataStorage->Close();
	    
	return 0;
}
