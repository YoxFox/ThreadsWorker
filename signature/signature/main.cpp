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

DataBuffer dataSource(10, 1024*1024);
DataBuffer resultStorage(10, 1024*1024);

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
            if (dataSource.isFullAvailable()) {
                break;
            }

            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::lock_guard<std::mutex> lock(sd_mutex);

        std::shared_ptr<DataUnit> unit = dataSource.producer_popWait();
        unit->id = ++sourceDataId;
        
        int * val = reinterpret_cast<int*>(unit->ptr);
        *val = rand() % 1000;

        BLOG << "Created id: " << unit->id << ", value: " << *val << ELOG;

        dataSource.producer_push(unit);
    }

    dataSource.clear();
}

void takeDataFromResult()
{
    while (true) {
        std::shared_ptr<const DataUnit> unit = resultStorage.consumer_popWait();
        int * val = reinterpret_cast<int*>(unit->ptr);

        BLOG << "Received id: " << unit->id << ", value: " << *val << ELOG;

        // Some manipulations
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        resultStorage.consumer_push(unit);

        if (unit->id > 49) {
            break;
        }
    }

    resultStorage.clear();
}

void workerTask()
{
    while (true) {
        BLOG << std::this_thread::get_id() << " | " << "begins" << ELOG;

        std::shared_ptr<DataUnit> result_unit = resultStorage.producer_popWait();

        if (!result_unit) {
            break;
        }

        BLOG << std::this_thread::get_id() << " | " << "gotten result unit" << ELOG;

        std::shared_ptr<const DataUnit> task_unit = dataSource.consumer_popWait();

        if (!task_unit) {
            resultStorage.producer_push(result_unit);
            break;
        }

        BLOG << std::this_thread::get_id() << " | " << "gotten souce unit" << ELOG;

        int * task_val_ptr = reinterpret_cast<int*>(task_unit->ptr);
        int * result_val_ptr = reinterpret_cast<int*>(result_unit->ptr);

        *result_val_ptr = *task_val_ptr + 5;
        result_unit->id = task_unit->id;

        // Very difficult task
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        dataSource.consumer_push(task_unit);
        resultStorage.producer_push(result_unit);

        BLOG << std::this_thread::get_id() << " | " << "ends" << ELOG;
    }

    BLOG << "===> Task is done for " << std::this_thread::get_id() << ELOG;
}

int main()
{
    srand(time(NULL));

    std::thread tc(createDataToSource);
    std::thread tr(takeDataFromResult);

    std::thread t1(workerTask), t2(workerTask), t3(workerTask), t4(workerTask), t5(workerTask);

    tc.join(); tr.join();
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
}