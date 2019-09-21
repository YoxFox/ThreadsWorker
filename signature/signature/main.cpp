﻿// main.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

/*
#include <iostream>

#include "types/doublequeue.h"

int main()
{
    std::cout << "Hello World!\n";

	DoubleQueue dq(5);

    std::shared_ptr<DataUnit> duPtr(new DataUnit(nullptr, 1, 52));

    std::cout << duPtr.use_count()<< "\n";

	dq.fPush(duPtr);

    std::cout << duPtr.use_count() << "\n";

    std::shared_ptr<DataUnit> duPtr_2 = dq.fPop();

	std::cout << duPtr_2->id << " " << duPtr_2.use_count() << "\n";
}

*/

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.




//  TESTS =================================

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <queue>
#include <time.h> 

#include "types/databuffer.h"
#include "services/filereaderbyparts.h"
#include "services/filewriterbyparts.h"
#include "services/workers/md5hashworker.h"

std::shared_ptr<twPro::FileReaderByParts> fileReader;
std::shared_ptr<twPro::FileWriterByParts> fileWriter;

std::shared_ptr<twPro::DataBuffer> dataSourcePtr;
std::shared_ptr<twPro::DataBuffer> resultStoragePtr;
std::shared_ptr<twPro::IWorker> worker;

std::atomic_bool stopFlag = false;

int sourceDataId = 0;
std::mutex sd_mutex;

static std::mutex log_mutex;
#define BLOG { std::lock_guard<std::mutex> lock(log_mutex); std::cout 
#define ELOG "\n";}

void createDataFromFile()
{
    BLOG << "Reader started" << ELOG;
    fileReader->work(stopFlag);
    BLOG << "Reader ended" << ELOG;
}

void writeDataToFile()
{
    BLOG << "Writer started" << ELOG;
    fileWriter->work(stopFlag);
    BLOG << "Writer ended" << ELOG;
}

void work() {
    BLOG << "=== START === worker thread: " << std::this_thread::get_id() << ELOG;
    worker->work(stopFlag);
    BLOG << "=== END === worker thread: " << std::this_thread::get_id() << ELOG;
}

void controlThread()
{
    BLOG << "Begin control" << ELOG;

    std::shared_ptr<EventHandler<unsigned long long>> eHandler_reader(new EventHandler<unsigned long long>(5));
    std::shared_ptr<EventHandler<unsigned long long>> eHandler_writer(new EventHandler<unsigned long long>(5));
    
    unsigned long long totalData = fileReader->totalData();
    BLOG << "File length: " << totalData << ELOG;

    unsigned long long totalWriteData = ((totalData / (1024*1024)) + 1) * 32;
    BLOG << "File write length: " << totalWriteData << ELOG;

    fileReader->setEventHandler_currentProducedData(eHandler_reader);
    fileWriter->setEventHandler_currentConsumedData(eHandler_writer);

    std::thread tc(createDataFromFile);
    std::thread tr(writeDataToFile);

    BLOG << "Reader listening" << ELOG;

    eHandler_reader->listen([&totalData](const HEvent<unsigned long long> & _event) -> bool {
        if (_event.value >= totalData) {
            return true;
        }
        return false;
    });

    BLOG << "Writer listening" << ELOG;

    eHandler_writer->listen([&totalWriteData, eHandler_writer](const HEvent<unsigned long long> & _event) -> bool {
        BLOG << "Writer listening, current consumed data length:" << _event.value << ", eQueueSize: " << eHandler_writer->currentQueueSize() << ELOG;
        if (_event.value >= totalWriteData) {
            return true;
        }
        return false;
    });

    BLOG << "End of the listening" << ELOG;

    stopFlag = true;

    dataSourcePtr->clear();
    dataSourcePtr.reset();

    BLOG << "Data source was cleared" << ELOG;

    resultStoragePtr->clear();
    resultStoragePtr.reset();

    BLOG << "Result storage was cleared" << ELOG;

    worker.reset();
    fileReader.reset();
    fileWriter.reset();

    BLOG << "Waiting thread ends" << ELOG;

    tc.join(); tr.join();

    BLOG << "End control" << ELOG;
}

int main()
{
    srand(time(NULL));

    dataSourcePtr.reset(new twPro::DataBuffer(20, 1024 * 1024));
    resultStoragePtr.reset(new twPro::DataBuffer(20, 32));
    worker.reset(new twPro::MD5HashWorker(dataSourcePtr, resultStoragePtr));

    fileReader.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\alita_film.mkv", dataSourcePtr));
    //fileReader.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\boost_1_71_0.7z", dataSourcePtr));
    //fileReader.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\!test_data.txt", dataSourcePtr));
    //fileReader.reset(new twPro::FileReaderByParts("H:\\alita_film.mkv", dataSourcePtr));

    //fileWriter.reset(new twPro::FileWriterByParts("H:\\alita_film_copy.mkv", resultStoragePtr));
    fileWriter.reset(new twPro::FileWriterByParts("C:\\Users\\YoxFox\\Downloads\\!data.sign", resultStoragePtr));

    std::thread ct(controlThread);

    std::thread t1(work), t2(work), t3(work), t4(work), t5(work),
    t6(work), t7(work), t8(work), t9(work), t10(work);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    t6.join(); t7.join(); t8.join(); t9.join(); t10.join();

    ct.join();

    //system("pause");
}