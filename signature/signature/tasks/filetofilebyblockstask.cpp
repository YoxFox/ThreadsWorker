#include "filetofilebyblockstask.h"

#include "../services/threadpool.h"
#include "../system/interactivefactory.h"

#include <sstream> 

namespace twPro {

    static size_t OPTIMAL_NOTIFIER_QUEUE_SIZE = 5;

    FileToFileByBlocksTask::FileToFileByBlocksTask(const FileToFileByBlocksTask_params & _params) noexcept : 
        m_params(_params), m_worker(nullptr)
    {
    }

    FileToFileByBlocksTask::~FileToFileByBlocksTask() noexcept
    {
    }
    
    // NOTE: We don't separate this method by methods because this is more readable and explicit way to show all step by step
    // It has to look like a script (scenario)

    void FileToFileByBlocksTask::run(std::atomic_bool & _stopFlag)
    {
        // We forbid to run it from different places at the time
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_params.blockSize < 1) {
            throw std::runtime_error("Block size can't be less than 1");
        } else if (m_params.resultFilePath.empty() || m_params.sourceFilePath.empty()) {
            throw std::runtime_error("File path can't be empty");
        }

        // === SETUP RESOURCES  ===

        ThreadPool tPool;
        size_t curAvailableThreads = tPool.countMaxAvailableThreads();

        m_sourceBufferPtr.reset(new twPro::DataBuffer(2*curAvailableThreads, m_params.blockSize));
        m_resultBufferPtr.reset(new twPro::DataBuffer(2*curAvailableThreads, m_worker->maxProducingDataUnitSizeByConsumingDataUnitSize(m_params.blockSize)));

        m_worker->setConsumerBuffer(m_sourceBufferPtr);
        m_worker->setProducerBuffer(m_resultBufferPtr);

        m_sourceFilePtr.reset(new twPro::FileReaderByParts(m_params.sourceFilePath));
        m_sourceFilePtr->setProducerBuffer(m_sourceBufferPtr);

        m_resultFilePtr.reset(new twPro::FileWriterByParts(m_params.resultFilePath));
        m_resultFilePtr->setConsumerBuffer(m_resultBufferPtr);

        size_t totalData = m_sourceFilePtr->totalData();
        size_t totalWriteDataUnits = ((totalData / m_params.blockSize) + (totalData % m_params.blockSize > 0 ? 1 : 0));

        // === PRINT MAIN INFO ===

        B_INFO << "Task block size: " << m_params.blockSize << " bytes" << E_INFO;
        B_INFO << "Source data length: " << totalData << " bytes" << E_INFO;

        // === RUN WORKERS IN THREADS ===

        std::atomic_bool stopFlag = false;
        _pResult ret = true;

        auto fileReaderJob = [this, &stopFlag, &ret]() {

            try {
                m_sourceFilePtr->work(stopFlag);
            }
            catch (const std::exception& ex) {
                ret = false;
                ret.error = ex.what();
            }
            catch (const std::string& ex) {
                ret = false;
                ret.error = ex;
            }
            catch (...) {
                ret = false;
                ret.error = "File reader returns unknown error";
            }

            if (!ret) {
                stopFlag = true;
            }

        };

        auto fileWriterJob = [this, &stopFlag, &ret]() {

            try {
                m_resultFilePtr->work(stopFlag);
            }
            catch (const std::exception& ex) {
                ret = false;
                ret.error = ex.what();
            }
            catch (const std::string& ex) {
                ret = false;
                ret.error = ex;
            }
            catch (...) {
                ret = false;
                ret.error = "File writer returns unknown error";
            }

            if (!ret) {
                stopFlag = true;
            }

        };

        auto workerJob = [this, &stopFlag, &ret]() {

            try {
                m_worker->work(stopFlag);
            }
            catch (const std::exception& ex) {
                ret = false;
                ret.error = ex.what();
            }
            catch (const std::string& ex) {
                ret = false;
                ret.error = ex;
            }
            catch (...) {
                ret = false;
                ret.error = "Worker returns unknown error";
            }

            if (!ret) {
                stopFlag = true;
            }

        };

        tPool.poolTask(fileReaderJob);
        tPool.poolTask(fileWriterJob);

        if (curAvailableThreads <= 1) {
            tPool.poolTask(workerJob);
        } else {
            for (size_t i = 1; i <= curAvailableThreads - 1; ++i) { tPool.poolTask(workerJob); }
        }

        // === SETUP NOTIFIERS ===

        twPro::DataChannel ch;
        auto writerNotifier = ch.createNotifier<size_t>(OPTIMAL_NOTIFIER_QUEUE_SIZE);
        auto progress = interactive()->createProgressBar();

        m_resultFilePtr->setNotifier_currentConsumedDataUnits(writerNotifier);

        writerNotifier->setCallBack([&totalWriteDataUnits, &stopFlag, &progress](const size_t & _val) {

            progress->notify(IInteractive::Progress("Progress", _val, totalWriteDataUnits));
            
            if (_val >= totalWriteDataUnits) {
                stopFlag = true;
            }

        });

        // === LINSTENING ===

        ch.listen([&stopFlag, &_stopFlag]() -> bool {
            return stopFlag || _stopFlag;
        });

        if (!ret) {
            interactive()->pushMessage(ret.error.empty() ? ret.error : "Unknown internal error",
                IInteractive::MessageType::ERROR_m);
        }
        else {
            // 100%
            progress->notify(IInteractive::Progress("Progress", 100, 100));
        }

        stopFlag = true;

        // === CLEAR ALL DATA AND SERVICES ===

        m_sourceBufferPtr->clear();
        m_sourceBufferPtr.reset();

        m_resultBufferPtr->clear();
        m_resultBufferPtr.reset();

        m_worker.reset();
        m_sourceFilePtr.reset();
        m_resultFilePtr.reset();

        tPool.join();
    }

}
