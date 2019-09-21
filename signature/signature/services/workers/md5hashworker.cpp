#include "md5hashworker.h"

#include "../../3rdparty/md5.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;
    static size_t OUTPUT_DATA_LENGTH = 32; // This worker creates 32 hex type values of the MD5 hash, not 16 bytes of the MD5 hash value

    MD5HashWorker::MD5HashWorker(const std::shared_ptr<DataBuffer> & _dataProducer, const std::shared_ptr<DataBuffer> & _resultStorage) :
        m_isStopped(false),
        m_dataProducer(_dataProducer), m_resultStorage(_resultStorage)
    {
    }

    MD5HashWorker::~MD5HashWorker()
    {
    }

    void MD5HashWorker::work(std::atomic_bool & _stopFlag)
    {
        m_isStopped.store(false);

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> result_unit = m_resultStorage->producer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!result_unit) {
                continue;
            }

            if (result_unit->size < OUTPUT_DATA_LENGTH) {
                // throw exception about output length
                break;
            }

            std::shared_ptr<twPro::DataUnit> task_unit = m_dataProducer->consumer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!task_unit) {
                m_resultStorage->producer_pushNotUsed(result_unit);
                continue;
            }

            calculateHashValue(task_unit, result_unit);

            result_unit->id = task_unit->id;
            result_unit->dataSize = OUTPUT_DATA_LENGTH;

            m_dataProducer->consumer_push(task_unit);
            m_resultStorage->producer_push(result_unit);
        }
    }

    void MD5HashWorker::calculateHashValue(const std::shared_ptr<const twPro::DataUnit>& _dataUnit, const std::shared_ptr<twPro::DataUnit>& _resultUnit) const
    {
        char* inputCharArray = reinterpret_cast<char*>(_dataUnit->ptr);
        char* outputCharArray = reinterpret_cast<char*>(_resultUnit->ptr);

        twPro_3rd::bzflag::MD5 hash_class(inputCharArray, _dataUnit->dataSize);
        hash_class.hexdigest(outputCharArray);
    }

}