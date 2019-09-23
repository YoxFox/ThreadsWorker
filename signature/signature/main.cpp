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

std::shared_ptr<twPro::FileReaderByParts> dataProducer;
std::shared_ptr<twPro::FileWriterByParts> dataConsumer;
std::shared_ptr<twPro::IWorker> worker;

std::shared_ptr<twPro::DataBuffer> dataSourcePtr;
std::shared_ptr<twPro::DataBuffer> resultStoragePtr;

std::atomic_bool stopFlag = false;

int sourceDataId = 0;
std::mutex sd_mutex;

static std::mutex log_mutex;
#define BLOG { std::lock_guard<std::mutex> lock(log_mutex); std::cout 
#define ELOG "\n";}

void doDataProducing()
{
    BLOG << "Reader started" << ELOG;
    dataProducer->work(stopFlag);
    BLOG << "Reader ended" << ELOG;
}

void doDataConsuming()
{
    BLOG << "Writer started" << ELOG;
    dataConsumer->work(stopFlag);
    BLOG << "Writer ended" << ELOG;
}

void work() {
    BLOG << "=== START === worker thread: " << std::this_thread::get_id() << ELOG;
    worker->work(stopFlag);
    BLOG << "=== END === worker thread: " << std::this_thread::get_id() << ELOG;
}

void controlThread()
{
    srand(time(0));

    BLOG << "++++++++++ Begin control ++++++++++" << ELOG;
    stopFlag = false;

    // === SET RESOURCES  ===

    size_t blockSize = 64 + std::rand() % 1024*1024*10;
    BLOG << ">> Current block size: " << blockSize << ELOG;

    worker.reset(new twPro::MD5HashWorker());

    dataSourcePtr.reset(new twPro::DataBuffer(20, blockSize));
    resultStoragePtr.reset(new twPro::DataBuffer(20, worker->maxProducingDataUnitSizeByConsumingDataUnitSize(blockSize)));

    worker->setConsumerBuffer(dataSourcePtr);
    worker->setProducerBuffer(resultStoragePtr);

    dataProducer.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\alita_film.mkv"));
    //fileReader.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\boost_1_71_0.7z", dataSourcePtr));
    //fileReader.reset(new twPro::FileReaderByParts("C:\\Users\\YoxFox\\Downloads\\!test_data.txt", dataSourcePtr));
    //fileReader.reset(new twPro::FileReaderByParts("H:\\alita_film.mkv", dataSourcePtr));
    dataProducer->setProducerBuffer(dataSourcePtr);

    //fileWriter.reset(new twPro::FileWriterByParts("H:\\alita_film_copy.mkv", resultStoragePtr));
    dataConsumer.reset(new twPro::FileWriterByParts("C:\\Users\\YoxFox\\Downloads\\!data.sign"));
    dataConsumer->setConsumerBuffer(resultStoragePtr);

    // === SET HANDLERS ===

    std::shared_ptr<EventHandler<unsigned long long>> eHandler_reader(new EventHandler<unsigned long long>(5));
    std::shared_ptr<EventHandler<unsigned long long>> eHandler_writer(new EventHandler<unsigned long long>(5));
    
    size_t totalData = dataProducer->totalData();
    BLOG << "File length: " << totalData << ELOG;

    size_t totalWriteDataUnits = ((totalData / blockSize) + (totalData % blockSize > 0 ? 1 : 0));
    BLOG << "File write units: " << totalWriteDataUnits << ELOG;

    dataProducer->setEventHandler_currentProducedDataUnits(eHandler_reader);
    dataConsumer->setEventHandler_currentConsumedDataUnits(eHandler_writer);

    // === RUN SOURCES ANS RESULTS ===

    std::thread tc(doDataProducing);
    std::thread tr(doDataConsuming);

    // === RUN WORKERS ===

    std::thread t1(work), t2(work), t3(work), t4(work), t5(work),
    t6(work), t7(work), t8(work), t9(work), t10(work);

    // === RESULTS LISTENING ===

    BLOG << "Reader listening" << ELOG;

    eHandler_reader->listen([&totalWriteDataUnits](const HEvent<unsigned long long> & _event) -> bool {
        if (_event.value >= totalWriteDataUnits) {
            return true;
        }
        return false;
    });

    BLOG << "Writer listening" << ELOG;

    eHandler_writer->listen([&totalWriteDataUnits, eHandler_writer](const HEvent<unsigned long long> & _event) -> bool {
        BLOG << "Writer listening, current consumed data units:" << _event.value << ", eQueueSize: " << eHandler_writer->currentQueueSize() << ELOG;
        if (_event.value >= totalWriteDataUnits) {
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
    dataProducer.reset();
    dataConsumer.reset();

    BLOG << "Waiting thread ends" << ELOG;

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    t6.join(); t7.join(); t8.join(); t9.join(); t10.join();

    tc.join(); tr.join();

    BLOG << "++++++++++ End control ++++++++++\n" << ELOG;
}

#include "types/datachannel.h"

void stFoo(std::shared_ptr<twPro::Notifier<int>> _notifier)
{
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        _notifier->notify(i);
    }
}

void stFoo_ch(std::shared_ptr<twPro::Notifier<char>> _notifier)
{
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        _notifier->notify('T');
    }
}

int main()
{
    twPro::DataChannel ch;
    std::shared_ptr<twPro::Notifier<int>> intNotifier = ch.createNotifier<int>();
    std::shared_ptr<twPro::Notifier<char>> charNotifier = ch.createNotifier<char>();

    std::atomic_bool stopVal = false;

    intNotifier->setCallBack([&stopVal](const int & _val) {
        BLOG << "HOP: " << _val << ELOG;
        if (_val >= 9) {
            stopVal = true;
        }
    });

    charNotifier->setCallBack([&stopVal](const char & _val) {
        BLOG << "HIP: " << _val << ELOG;
    });

    std::thread someThread(stFoo, intNotifier);
    std::thread someThread_2(stFoo_ch, charNotifier);

    ch.listen(stopVal);

    someThread.join();
    someThread_2.join();

    /*
    unsigned int testsCount = 100;
    for (unsigned int idx = 0; idx < testsCount; ++idx) {
        std::thread ct(controlThread);
        ct.join();
    }
    */

    /*
    while (true) {
        std::thread ct(controlThread);
        ct.join();
    }
    */

    /*
    std::thread ct(controlThread);
    ct.join();
    */

    //system("pause");
}