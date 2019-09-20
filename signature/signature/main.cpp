// main.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
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

std::shared_ptr<twPro::DataBuffer> dataSourcePtr;
std::shared_ptr<twPro::DataBuffer> resultStoragePtr;
std::shared_ptr<twPro::IWorker> worker;

int sourceDataId = 0;
std::mutex sd_mutex;

std::mutex log_mutex;
#define BLOG { std::lock_guard<std::mutex> lock(log_mutex); std::cout 
#define ELOG "\n";}

void createDataToSource()
{
    while (true) {
        // Creating data

        if (sourceDataId > 49) {
            if (dataSourcePtr->isFullAvailable()) {
                break;
            }

            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::lock_guard<std::mutex> lock(sd_mutex);

        std::shared_ptr<twPro::DataUnit> unit = dataSourcePtr->producer_popWait().lock();
        unit->id = ++sourceDataId;
        
        int * val = reinterpret_cast<int*>(unit->ptr);
        *val = rand() % 1000;

        BLOG << "Created id: " << unit->id << ", value: " << *val << ELOG;

        dataSourcePtr->producer_push(unit);
    }

    dataSourcePtr->clear();
}

void createDataFromFile()
{
    twPro::FileReaderByParts fileReader("C:\\Users\\YoxFox\\Downloads\\alita_film.mkv", dataSourcePtr);
    //twPro::FileReaderByParts fileReader("C:\\Users\\YoxFox\\Downloads\\!test_data.txt", dataSourcePtr);
    //twPro::FileReaderByParts fileReader("H:\\alita_film.mkv", dataSourcePtr);
    fileReader.work();
}

void writeDataToFile()
{
    //twPro::FileWriterByParts fileReader("H:\\alita_film_copy.mkv", resultStoragePtr);
    twPro::FileWriterByParts fileReader("C:\\Users\\YoxFox\\Downloads\\!test_data.sign", resultStoragePtr);
    fileReader.work();
}

void takeDataFromResult()
{
    while (true) {
        std::shared_ptr<twPro::DataUnit> unit = resultStoragePtr->consumer_popWait().lock();

        BLOG << "Received id: " << unit->id << ", dataSize: " << unit->dataSize << ", mem size: " << unit->size << ELOG;

        // Some manipulations
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));

        resultStoragePtr->consumer_push(unit);

        if (unit->id > 49) {
            break;
        }
    }

    resultStoragePtr->clear();
}

void workerTask()
{
    while (true) {
        BLOG << std::this_thread::get_id() << " | " << "begins" << ELOG;

        std::shared_ptr<twPro::DataUnit> result_unit = resultStoragePtr->producer_popWait().lock();

        if (!result_unit) {
            break;
        }

        BLOG << std::this_thread::get_id() << " | " << "gotten result unit" << ELOG;

        std::shared_ptr<twPro::DataUnit> task_unit = dataSourcePtr->consumer_popWait().lock();

        if (!task_unit) {
            resultStoragePtr->producer_push(result_unit);
            break;
        }

        BLOG << std::this_thread::get_id() << " | " << "gotten souce unit" << ELOG;

 //       int * task_val_ptr = reinterpret_cast<int*>(task_unit->ptr);
 //       int * result_val_ptr = reinterpret_cast<int*>(result_unit->ptr);
 //       *result_val_ptr = *task_val_ptr + 5;

        std::memcpy(result_unit->ptr, task_unit->ptr, task_unit->dataSize);

        result_unit->id = task_unit->id;
        result_unit->dataSize = task_unit->dataSize;

        // Very difficult task
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));

        dataSourcePtr->consumer_push(task_unit);
        resultStoragePtr->producer_push(result_unit);

        BLOG << std::this_thread::get_id() << " | " << "ends" << ELOG;
    }

    BLOG << "===> Task is done for " << std::this_thread::get_id() << ELOG;
}

int main()
{
    srand(time(NULL));
    dataSourcePtr.reset(new twPro::DataBuffer(20, 1024 * 1024));
    resultStoragePtr.reset(new twPro::DataBuffer(20, 32));
    worker.reset(new twPro::MD5HashWorker(dataSourcePtr, resultStoragePtr));

    std::thread tc(createDataFromFile);
    std::thread tr(writeDataToFile);

//    std::thread tc(createDataToSource);
//    std::thread tr(takeDataFromResult);

    std::thread t1(&twPro::IWorker::work, worker), t2(&twPro::IWorker::work, worker), t3(&twPro::IWorker::work, worker), t4(&twPro::IWorker::work, worker), t5(&twPro::IWorker::work, worker),
    t6(&twPro::IWorker::work, worker), t7(&twPro::IWorker::work, worker), t8(&twPro::IWorker::work, worker), t9(&twPro::IWorker::work, worker), t10(&twPro::IWorker::work, worker);

    tc.join(); tr.join();
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    t6.join(); t7.join(); t8.join(); t9.join(); t10.join();
}