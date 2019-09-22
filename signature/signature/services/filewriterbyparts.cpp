#include "filewriterbyparts.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    FileWriterByParts::FileWriterByParts(const std::string & _filePath, const std::shared_ptr<twPro::DataBuffer> & _buffer) :
        m_consumedDataBlocks(0),
        m_consumedDataLength(0),
        m_buffer(_buffer)
    {
        m_stream.open(_filePath, std::ifstream::binary);

        if (!m_stream) {
            throw std::runtime_error("Can't open file to read: " + _filePath);
        }
    }

    FileWriterByParts::~FileWriterByParts()
    {
        m_stream.close();
    }

    void FileWriterByParts::work(std::atomic_bool & _stopFlag)
    {
        std::lock_guard<std::mutex> lock(work_mutex);

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> unit = m_buffer->consumer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!unit) {
                continue;
            }

            size_t offset = unit->id * unit->size;
            m_stream.seekp(offset, std::ios_base::beg);

            // write
            m_stream.write(reinterpret_cast<char*>(unit->ptr), unit->dataSize);

            ++m_consumedDataBlocks;
            m_consumedDataLength += unit->dataSize;
            m_buffer->consumer_push(unit);
            eventHandler_currentConsumedDataUnits_notify(HEvent<size_t>(m_consumedDataBlocks));
        }
    }

    size_t FileWriterByParts::currentConsumedData() const noexcept
    {
        return m_consumedDataLength.load();
    }

    size_t FileWriterByParts::currentConsumedDataUnits() const noexcept
    {
        return m_consumedDataBlocks;
    }
}
