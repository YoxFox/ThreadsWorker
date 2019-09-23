#include "filetofilebyblockstask.h"

#include "../services/threadpool.h"

#include <iostream>
static std::mutex log_mutex;
#define BLOG { std::lock_guard<std::mutex> lock(log_mutex); std::cout 
#define ELOG "\n";}

namespace twPro {

    FileToFileByBlocksTask::FileToFileByBlocksTask(const FileToFileByBlocksTask_params & _params) noexcept : 
        m_params(_params), m_worker(nullptr)
    {
    }

    FileToFileByBlocksTask::~FileToFileByBlocksTask()
    {
    }

    void FileToFileByBlocksTask::run(std::atomic_bool & _stopFlag)
    {
        // We forbid to run it from different places at the time
        std::lock_guard<std::mutex> lock(m_mutex);

        BLOG << "++++++++++ Begin control ++++++++++" << ELOG;

        std::atomic_bool stopFlag = false;

        // === SET RESOURCES  ===

        BLOG << ">>> Current block size: " << m_params.blockSize << " <<<" << ELOG;

        m_sourceBufferPtr.reset(new twPro::DataBuffer(20, m_params.blockSize));
        m_resultBufferPtr.reset(new twPro::DataBuffer(20, m_worker->maxProducingDataUnitSizeByConsumingDataUnitSize(m_params.blockSize)));

        m_worker->setConsumerBuffer(m_sourceBufferPtr);
        m_worker->setProducerBuffer(m_resultBufferPtr);

        m_sourceFilePtr.reset(new twPro::FileReaderByParts(m_params.sourceFilePath));
        m_sourceFilePtr->setProducerBuffer(m_sourceBufferPtr);

        m_resultFilePtr.reset(new twPro::FileWriterByParts(m_params.resultFilePath));
        m_resultFilePtr->setConsumerBuffer(m_resultBufferPtr);

        // === SET NOTIFIERS ===

        twPro::DataChannel ch;
        std::shared_ptr<twPro::Notifier<size_t>> readerNotifier = ch.createNotifier<size_t>(5);
        std::shared_ptr<twPro::Notifier<size_t>> writerNotifier = ch.createNotifier<size_t>(5);

        size_t totalData = m_sourceFilePtr->totalData();
        BLOG << "File length: " << totalData << ELOG;

        size_t totalWriteDataUnits = ((totalData / m_params.blockSize) + (totalData % m_params.blockSize > 0 ? 1 : 0));
        BLOG << "File write units: " << totalWriteDataUnits << ELOG;

        m_sourceFilePtr->setNotifier_currentProducedDataUnits(readerNotifier);
        m_resultFilePtr->setNotifier_currentConsumedDataUnits(writerNotifier);

        // === RUN WORKERS IN THREADS ===

        auto fileReaderJob = [this, &stopFlag]() {
            m_sourceFilePtr->work(stopFlag);
        };

        auto fileWriterJob = [this, &stopFlag]() {
            m_resultFilePtr->work(stopFlag);
        };

        auto workerJob = [this, &stopFlag]() {
            m_worker->work(stopFlag);
        };

        ThreadPool tPool;

        tPool.poolTask(fileReaderJob);
        tPool.poolTask(fileWriterJob);

        size_t curAvailableThreads = tPool.countMaxAvailableThreads();

        if (curAvailableThreads <= 1) {
            tPool.poolTask(workerJob);
        } else {
            for (size_t i = 1; i <= curAvailableThreads - 1; ++i) { tPool.poolTask(workerJob); }
        }

        // === RESULTS LISTENING ===

        readerNotifier->setCallBack([&totalWriteDataUnits](const size_t & _val) {
            if (_val >= totalWriteDataUnits) {
                BLOG << "Reader is done" << ELOG;
            }
        });

        writerNotifier->setCallBack([&totalWriteDataUnits, &stopFlag](const size_t & _val) {
            if (_val >= totalWriteDataUnits) {
                stopFlag = true;
                BLOG << "Writer is done" << ELOG;
            }
        });

        BLOG << "Begin of the listening" << ELOG;

        ch.listen([&stopFlag, &_stopFlag]() -> bool {
            return stopFlag || _stopFlag;
        });

        stopFlag = true;

        BLOG << "End of the listening" << ELOG;

        m_sourceBufferPtr->clear();
        m_sourceBufferPtr.reset();

        BLOG << "Data source was cleared" << ELOG;

        m_resultBufferPtr->clear();
        m_resultBufferPtr.reset();

        BLOG << "Result storage was cleared" << ELOG;

        m_worker.reset();
        m_sourceFilePtr.reset();
        m_resultFilePtr.reset();

        BLOG << "Waiting thread ends" << ELOG;

        tPool.join();

        BLOG << "++++++++++ End control ++++++++++\n" << ELOG;
    }

}
