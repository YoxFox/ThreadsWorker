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

    void BaseBlockWorker::work(std::atomic_bool & _stopFlag)
    {
        m_isStopped.store(false);

        if (!m_dataSource || !m_resultStorage) {
            throw std::runtime_error("Internal error: data buffer doesn't exist");
        }

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> result_unit = m_resultStorage->producer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!result_unit) {
                continue;
            }

            std::shared_ptr<twPro::DataUnit> task_unit = m_dataSource->consumer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!task_unit) {
                m_resultStorage->producer_pushNotUsed(result_unit);
                continue;
            }

            if (task_unit->dataSize > maxConsumingDataUnitSize()) {
                m_resultStorage->producer_pushNotUsed(result_unit);
                throw std::runtime_error("Internal error: incorrect buffer size for source data");
            }

            size_t maxProducingDataUnitSize = maxProducingDataUnitSizeByConsumingDataUnitSize(task_unit->dataSize);
            if (result_unit->size < maxProducingDataUnitSize) {
                throw std::runtime_error("Internal error: incorrect buffer size for result data");
            }

            auto returnResources = [this, &result_unit, &task_unit]() {
                m_resultStorage->producer_pushNotUsed(result_unit);
                m_dataSource->consumer_pushNotUsed(task_unit);
            };

            try {
                doBlockWork(task_unit, result_unit);
            }
            catch (const std::exception& ex) {

                returnResources();
                throw ex;
            }
            catch (const std::string& ex) {
                returnResources();
                throw ex;
            }
            catch (...) {
                returnResources();
                throw std::runtime_error("Internal error: the worker returns unknown error");
            }

            result_unit->id = task_unit->id;
            result_unit->dataSize = maxProducingDataUnitSize;

            m_dataSource->consumer_push(task_unit);
            m_resultStorage->producer_push(result_unit);
        }
    }

    void BaseBlockWorker::setConsumerBuffer(const std::shared_ptr<twPro::DataBuffer>& _cBuffer) noexcept
    {
        m_dataSource = _cBuffer;
    }

    void BaseBlockWorker::setProducerBuffer(const std::shared_ptr<twPro::DataBuffer>& _pBuffer) noexcept
    {
        m_resultStorage = _pBuffer;
    }

}