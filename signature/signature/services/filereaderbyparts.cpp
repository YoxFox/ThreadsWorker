#include "filereaderbyparts.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    FileReaderByParts::FileReaderByParts(const std::string & _filePath, const std::shared_ptr<twPro::DataBuffer> & _buffer) :
        m_isDone(false), m_isStopped(true), m_idPart(0), m_producedDataLength(0), m_fileLength(0),
        m_buffer(_buffer), m_stream(_filePath, std::ifstream::binary)
    {
        if (!m_stream) {
            throw std::runtime_error("Can't open file to read: " + _filePath);
        }

        // get length
        m_stream.seekg(0, std::ios_base::end);
        m_fileLength = m_stream.tellg();
    }

    FileReaderByParts::~FileReaderByParts()
    {
    }

    void FileReaderByParts::work(std::atomic_bool & _stopFlag)
    {
        std::lock_guard<std::mutex> lock(work_mutex);

        if (isDone()) {
            return;
        }

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> unit = m_buffer->producer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!unit) {
                continue;
            }

            size_t offset = m_idPart * unit->size;

            if (offset >= m_fileLength) {
                m_buffer->producer_pushNotUsed(unit);
                break;
            }

            m_stream.seekg(offset, std::ios_base::beg);

            // get block length
            size_t availableDataSize = m_fileLength - offset;
            size_t blockLength = availableDataSize > unit->size ? unit->size : availableDataSize;

            // read
            m_stream.read(reinterpret_cast<char*>(unit->ptr), blockLength);

            unit->id = m_idPart;
            unit->dataSize = blockLength;

            m_buffer->producer_push(unit);

            ++m_idPart;
            m_producedDataLength += blockLength;
            eventHandler_currentProducedData_notify(HEvent<unsigned long long>(m_producedDataLength));
        }

        if (m_producedDataLength >= m_fileLength) {
            m_isDone = true;
        }
    }

    bool FileReaderByParts::isDone() const noexcept
    {
        return m_isDone.load();
    }

    unsigned long long FileReaderByParts::currentProducedData() const noexcept
    {
        return m_producedDataLength.load();
    }

    unsigned long long FileReaderByParts::totalData() const noexcept
    {
        return m_fileLength.load();
    }

}
