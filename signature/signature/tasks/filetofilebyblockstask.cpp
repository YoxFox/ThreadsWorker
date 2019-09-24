#include "filetofilebyblockstask.h"

#include "../services/threadpool.h"
#include "../system/interactivefactory.h"

#include <sstream> 

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

        B_INFO << "++++++++++ Begin control ++++++++++" << E_INFO;

        std::atomic_bool stopFlag = false;

        // === SET RESOURCES  ===

        B_INFO << ">>> Current block size: " << m_params.blockSize << " <<<" << E_INFO;

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
        B_INFO << "File length: " << totalData << E_INFO;

        size_t totalWriteDataUnits = ((totalData / m_params.blockSize) + (totalData % m_params.blockSize > 0 ? 1 : 0));
        B_INFO << "File write units: " << totalWriteDataUnits << E_INFO;

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
                B_INFO << "Reader is done" << E_INFO;
            }
        });

        std::shared_ptr<twPro::Notifier<IInteractive::Progress>> progress = interactive()->createProgressBar();

        writerNotifier->setCallBack([&totalWriteDataUnits, &stopFlag, &progress](const size_t & _val) {

            progress->notify(IInteractive::Progress("Signing", _val, totalWriteDataUnits));
            
            if (_val >= totalWriteDataUnits) {
                stopFlag = true;
                B_INFO << "Writer is done" << E_INFO;
            }
        });

        B_INFO << "Begin of the listening" << E_INFO;

        ch.listen([&stopFlag, &_stopFlag]() -> bool {
            return stopFlag || _stopFlag;
        });

        progress->notify(IInteractive::Progress("Signing", 100, 100));
        stopFlag = true;

        B_INFO << "End of the listening" << E_INFO;

        m_sourceBufferPtr->clear();
        m_sourceBufferPtr.reset();

        B_INFO << "Data source was cleared" << E_INFO;

        m_resultBufferPtr->clear();
        m_resultBufferPtr.reset();

        B_INFO << "Result storage was cleared" << E_INFO;

        m_worker.reset();
        m_sourceFilePtr.reset();
        m_resultFilePtr.reset();

        B_INFO << "Waiting thread ends" << E_INFO;

        tPool.join();

        B_INFO << "++++++++++ End control ++++++++++\n" << E_INFO;
    }

}
