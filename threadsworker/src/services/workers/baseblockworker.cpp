#include "baseblockworker.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    BaseBlockWorker::BaseBlockWorker() noexcept :
        m_isStopped(false),
        m_dataSource(nullptr), m_resultStorage(nullptr)
    {
    }

    BaseBlockWorker::~BaseBlockWorker() noexcept
    {
    }

    Result<WORKER_CODES> BaseBlockWorker::work(std::atomic_bool & _stopFlag) noexcept
    {
        m_isStopped.store(false);

        if (!m_dataSource || !m_resultStorage) {
            return { WORKER_CODES::INTERNAL_ERROR, "Internal error: data buffer doesn't exist" };
        }

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> result_unit = m_resultStorage->left_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!result_unit) {
                continue;
            }

            std::shared_ptr<twPro::DataUnit> task_unit = m_dataSource->right_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!task_unit) {
                m_resultStorage->left_push(result_unit);
                continue;
            }

            auto returnResources = [this, &result_unit, &task_unit]() {
                m_resultStorage->left_push(result_unit);
                m_dataSource->right_push(task_unit);
            };

            if (task_unit->dataSize > maxConsumingDataUnitSize()) {
                returnResources();
                return { WORKER_CODES::INTERNAL_ERROR, "Internal error: incorrect buffer size for source data" };
            }

            size_t maxProducingDataUnitSize = maxProducingDataUnitSizeByConsumingDataUnitSize(task_unit->dataSize);
            if (result_unit->size < maxProducingDataUnitSize) {
                returnResources();
                return { WORKER_CODES::INTERNAL_ERROR, "Internal error : incorrect buffer size for result data" };
            }

            Result<WORKER_CODES> ret = doBlockWork(task_unit, result_unit);

            if (!ret) {
                returnResources();
                return ret;
            }

            result_unit->id = task_unit->id;
            result_unit->dataSize = maxProducingDataUnitSize;

            m_dataSource->left_push(task_unit);
            m_resultStorage->right_push(result_unit);
        }

        return WORKER_CODES::OK;
    }

    void BaseBlockWorker::setConsumerBuffer(const std::shared_ptr<twPro::LRDataBuffer>& _cBuffer) noexcept
    {
        m_dataSource = _cBuffer;
    }

    void BaseBlockWorker::setProducerBuffer(const std::shared_ptr<twPro::LRDataBuffer>& _pBuffer) noexcept
    {
        m_resultStorage = _pBuffer;
    }

}