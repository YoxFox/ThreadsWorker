#include "filereaderbyparts.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    FileReaderByParts::FileReaderByParts(const std::string & _filePath) :
        m_isDone(false), m_isStopped(true), m_idPart(0), m_producedDataLength(0), m_fileLength(0),
        m_buffer(nullptr)
    {
        try {
            m_stream.open(_filePath, std::ifstream::binary);
        }
        catch (std::ifstream::failure e) {
            throw e;
        }

        if (!m_stream) {
            throw std::runtime_error("Can't open file to read: " + _filePath);
        }

        try {
            // get length
            m_stream.seekg(0, std::ios_base::end);
            m_fileLength = m_stream.tellg();
        }
        catch (std::ifstream::failure e) {
            throw e;
        }
    }

    FileReaderByParts::~FileReaderByParts() noexcept
    {
        // Wait to release
        std::lock_guard<std::mutex> lock(work_mutex);

        try {
            if (m_stream) {
                m_stream.close();
            }
        }
        catch (std::ifstream::failure e) {
            // TODO: Create solution
            /* DO NOTHING = We can't to do anything, maybe */
        }
    }

    void FileReaderByParts::setProducerBuffer(const std::shared_ptr<twPro::LRDataBuffer>& _buffer) noexcept
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        m_buffer = _buffer;
    }

    void FileReaderByParts::work(std::atomic_bool & _stopFlag)
    {
        std::lock_guard<std::mutex> lock(work_mutex);

        if (isDone()) {
            return;
        }

        if (!m_buffer) {
            throw std::runtime_error("Internal error: data buffer doesn't exist");
        }

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> unit = m_buffer->left_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!unit) {
                continue;
            }

            size_t offset = m_idPart * unit->size;

            if (offset >= m_fileLength) {
                m_buffer->left_push(unit);
                break;
            }

            // get block length
            size_t availableDataSize = m_fileLength - offset;
            size_t blockLength = availableDataSize > unit->size ? unit->size : availableDataSize;

            try {
                // Set pointer to offset value
                m_stream.seekg(offset, std::ios_base::beg);

                // read
                m_stream.read(reinterpret_cast<char*>(unit->ptr), blockLength);
            }
            catch (std::ifstream::failure e) {
                m_buffer->left_push(unit);
                throw e;
            }

            unit->id = m_idPart;
            unit->dataSize = blockLength;

            m_buffer->right_push(unit);

            ++m_idPart;
            m_producedDataLength += blockLength;
            currentProducedDataUnits_notify(m_idPart);
        }

        if (m_producedDataLength >= m_fileLength) {
            m_isDone = true;
        }
    }

    bool FileReaderByParts::isDone() const noexcept
    {
        return m_isDone.load();
    }

    size_t FileReaderByParts::currentProducedData() const noexcept
    {
        return m_producedDataLength.load();
    }

    size_t FileReaderByParts::totalData() const noexcept
    {
        return m_fileLength.load();
    }

    size_t FileReaderByParts::currentProducedDataUnits() const noexcept
    {
        return m_idPart;
    }

}
