#include "filereaderbyparts.h"

#include <iostream>

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    FileReaderByParts::FileReaderByParts(const std::string & _filePath, const std::shared_ptr<twPro::DataBuffer> & _buffer) :
        m_isDone(false), m_isStopped(true), m_idPart(0), m_fileLength(0),
        m_buffer(_buffer), m_stream(_filePath, std::ifstream::binary)
    {
        if (!m_stream) {
            throw std::runtime_error("Can't open file to read: " + _filePath);
        }

        // get length
        m_stream.seekg(0, std::ios_base::end);
        m_fileLength = m_stream.tellg();
        std::cout << "File length: " << m_fileLength << "\n";
    }

    FileReaderByParts::~FileReaderByParts()
    {
    }

    void FileReaderByParts::work()
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        m_isStopped.store(false);

        while (true) {

            if (m_isStopped.load()) {
                return;
            }

            std::shared_ptr<twPro::DataUnit> unit = m_buffer->producer_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!unit) {
                // throw or ...
                break;
            }

            size_t offset = m_idPart * unit->size;

            if (offset >= m_fileLength) {
                break;
            }

            m_stream.seekg(offset, std::ios_base::beg);

            // get block length
            size_t availableDataSize = m_fileLength - offset;
            size_t blockLength = availableDataSize > unit->size ? unit->size : availableDataSize;

            // read
            m_stream.read(reinterpret_cast<char*>(unit->ptr), blockLength);

            if (blockLength < unit->size) {
                // Fill the tail by zeros
            }

            unit->id = m_idPart;
            unit->dataSize = blockLength;

            m_buffer->producer_push(unit);

            ++m_idPart;
        }

        // Make it if all data was readed
        m_isDone = true;
    }

    void FileReaderByParts::stop() noexcept
    {
        m_isStopped.store(true);
    }

    bool FileReaderByParts::isDone() const noexcept
    {
        return m_isDone.load();
    }

}
