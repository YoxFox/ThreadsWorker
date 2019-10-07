#include "sourcetoresulttemplatetask.h"

#include "../services/threadpool.h"
#include "../system/interactivefactory.h"

#undef ERROR

namespace twPro {

    SourceToResultTemplateTask::SourceToResultTemplateTask(const SourceToResultTemplateTask_params & _params) noexcept :
        m_params(_params), m_worker(nullptr)
    {
    }

    SourceToResultTemplateTask::~SourceToResultTemplateTask() noexcept
    {
    }

    Result<TASK_CODES> SourceToResultTemplateTask::run(std::atomic_bool & _stopFlag) noexcept
    {
        // We forbid to run it from different places at the time
        std::lock_guard<std::mutex> lock(m_mutex);

        Result<TASK_CODES> ret = true;

        ret = preCheck(m_params);

        if (!ret) {
            interactive()->pushMessage(!ret.text().empty() ? ret.text() : "Unknown internal error", IInteractive::MessageType::ERROR_m);
            return ret;
        }

        // === SETUP RESOURCES  ===

        ThreadPool tPool;
        size_t curAvailableThreads = tPool.countMaxAvailableThreads();

        auto sourceBuffer_RetVal = createBuffer(2 * curAvailableThreads, m_params.blockSize);

        if (!sourceBuffer_RetVal.result) {
            interactive()->pushMessage(!sourceBuffer_RetVal.result.text().empty() ? sourceBuffer_RetVal.result.text() : "Unknown internal error", IInteractive::MessageType::ERROR_m);
            return sourceBuffer_RetVal.result;
        }

        auto resultBuffer_RetVal = createBuffer(2 * curAvailableThreads, m_worker->maxProducingDataUnitSizeByConsumingDataUnitSize(m_params.blockSize));

        if (!resultBuffer_RetVal.result) {
            interactive()->pushMessage(!resultBuffer_RetVal.result.text().empty() ? resultBuffer_RetVal.result.text() : "Unknown internal error", IInteractive::MessageType::ERROR_m);
            return resultBuffer_RetVal.result;
        }

        m_sourceBufferPtr = sourceBuffer_RetVal.value;
        m_resultBufferPtr = resultBuffer_RetVal.value;

        ret = setupSources(m_sourceBufferPtr, m_resultBufferPtr);

        if (!ret) {
            interactive()->pushMessage(!ret.text().empty() ? ret.text() : "Unknown internal error", IInteractive::MessageType::ERROR_m);
            return ret;
        }

        m_worker->setConsumerBuffer(m_sourceBufferPtr);
        m_worker->setProducerBuffer(m_resultBufferPtr);

        // === RUN WORKERS IN THREADS ===

        std::atomic_bool stopFlag = false;

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
            interactive()->pushMessage(!ret.text().empty() ? ret.text() : "Unknown internal error", IInteractive::MessageType::ERROR_m);
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

        return ret;
    }

    twPro::RetVal<twPro::TASK_CODES, std::shared_ptr<twPro::LRDataBuffer>> SourceToResultTemplateTask::createBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize) const noexcept
    {
        RetVal<TASK_CODES, std::shared_ptr<twPro::LRDataBuffer>> retVal;
        retVal.result = true;

        try {
            retVal.value.reset(new twPro::LRDataBuffer(_bufferCapacity, _bufferUnitSize));
        }
        catch (std::exception e) {
            std::string text(e.what());
            retVal.result = std::make_tuple(TASK_CODES::ERROR, text.empty() ? "Buffer returns unexpected error" : text);
        }

        return retVal;
    }

}
