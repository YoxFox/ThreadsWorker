#include "md5hashworker.h"

#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

namespace twPro {

    static size_t OUTPUT_DATA_LENGTH = 32; // This worker creates 32 hex type values of the MD5 hash, not 16 bytes of the MD5 hash value

    MD5HashWorker::MD5HashWorker(const std::shared_ptr<DataBuffer> & _dataProducer, const std::shared_ptr<DataBuffer> & _resultStorage) :
        m_isStopped(false),
        m_dataProducer(_dataProducer), m_resultStorage(_resultStorage)
    {
    }

    MD5HashWorker::~MD5HashWorker()
    {
    }

    void MD5HashWorker::work()
    {
        m_isStopped.store(false);

        while (true) {

            //if (m_isStopped.load()) {
            //    return;
            //}

            std::shared_ptr<twPro::DataUnit> result_unit = m_resultStorage->producer_popWait().lock();

            if (!result_unit) {
                break;
            }

            if (result_unit->size < OUTPUT_DATA_LENGTH) {
                // throw exception about output length
                break;
            }

            std::shared_ptr<twPro::DataUnit> task_unit = m_dataProducer->consumer_popWait().lock();

            if (!task_unit) {
                m_resultStorage->producer_push(result_unit);
                break;
            }

            calculateHashValue(task_unit, result_unit);

            result_unit->id = task_unit->id;
            result_unit->dataSize = OUTPUT_DATA_LENGTH;

            m_dataProducer->consumer_push(task_unit);
            m_resultStorage->producer_push(result_unit);
        }
    }

    void MD5HashWorker::stop() noexcept
    {
        // DO NOTHING
    }

    void MD5HashWorker::calculateHashValue(const std::shared_ptr<const twPro::DataUnit>& _dataUnit, const std::shared_ptr<twPro::DataUnit>& _resultUnit) const
    {
        boost::uuids::detail::md5 hash;
        boost::uuids::detail::md5::digest_type digest;

        hash.process_bytes(_dataUnit->ptr, _dataUnit->dataSize);
        hash.get_digest(digest);

        const auto charDigest = reinterpret_cast<const char *>(&digest);
        char* res_ptr = reinterpret_cast<char*>(_resultUnit->ptr);
        boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type), res_ptr);
    }

}