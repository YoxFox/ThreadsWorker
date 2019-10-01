#include "sourcetoresulttemplatetask.h"

#include "../services/threadpool.h"
#include "../system/interactivefactory.h"

namespace twPro {

    SourceToResultTemplateTask::SourceToResultTemplateTask(const SourceToResultTemplateTask_params & _params) noexcept :
        m_params(_params), m_worker(nullptr)
    {
    }

    SourceToResultTemplateTask::~SourceToResultTemplateTask() noexcept
    {
    }

    void SourceToResultTemplateTask::run(std::atomic_bool & _stopFlag)
    {
        // We forbid to run it from different places at the time
        std::lock_guard<std::mutex> lock(m_mutex);

        preCheck(m_params);

        // === SETUP RESOURCES  ===

        ThreadPool tPool;
        size_t curAvailableThreads = tPool.countMaxAvailableThreads();

        m_sourceBufferPtr.reset(new twPro::LRDataBuffer(2 * curAvailableThreads, m_params.blockSize));
        m_resultBufferPtr.reset(new twPro::LRDataBuffer(2 * curAvailableThreads, m_worker->maxProducingDataUnitSizeByConsumingDataUnitSize(m_params.blockSize)));

        setupSources(m_sourceBufferPtr, m_resultBufferPtr);

        m_worker->setConsumerBuffer(m_sourceBufferPtr);
        m_worker->setProducerBuffer(m_resultBufferPtr);

        // === RUN WORKERS IN THREADS ===

        std::atomic_bool stopFlag = false;
        _pResult ret = true;

        auto fileReaderJob = prepareSourceJob(stopFlag, ret);
        auto fileWriterJob = prepareResultJob(stopFlag, ret);
        auto workerJob = prepareWorkerJob(m_worker, stopFlag, ret);

        tPool.poolTask(fileReaderJob);
        tPool.poolTask(fileWriterJob);

        if (curAvailableThreads <= 1) {
            tPool.poolTask(workerJob);
        }
        else {
            for (size_t i = 1; i <= curAvailableThreads - 1; ++i) { tPool.poolTask(workerJob); }
        }

        // === SETUP NOTIFIERS ===

        twPro::DataChannel ch;
        setupNotifiers(ch, stopFlag, ret);

        // === LINSTENING ===

        ch.listen([&stopFlag, &_stopFlag]() -> bool {
            return stopFlag || _stopFlag;
        });

        if (!ret) {
            interactive()->pushMessage(ret.error.empty() ? ret.error : "Unknown internal error",
                IInteractive::MessageType::ERROR_m);
        }

        stopFlag = true;

        // === CLEAR ALL DATA AND SERVICES ===

        clear();

        m_sourceBufferPtr->clear();
        m_sourceBufferPtr.reset();

        m_resultBufferPtr->clear();
        m_resultBufferPtr.reset();

        m_worker.reset();

        tPool.join();
    }

}
