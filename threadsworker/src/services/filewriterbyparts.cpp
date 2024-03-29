#include "filewriterbyparts.h"

namespace twPro {

    static long long UNIT_WAIT_TIMEOUT_MS = 100;

    FileWriterByParts::FileWriterByParts(const std::string & _filePath) :
        m_consumedDataBlocks(0),
        m_consumedDataLength(0),
        m_buffer(nullptr)
    {
        try {
            m_stream.open(_filePath, std::ofstream::binary);
        }
        catch (std::ofstream::failure e) {
            throw e;
        }

        if (!m_stream) {
            throw std::runtime_error("Can't open file to write: " + _filePath);
        }
    }

    FileWriterByParts::~FileWriterByParts() noexcept
    {
        // Wait to release
        std::lock_guard<std::mutex> lock(work_mutex);

        try {
            if (m_stream) {
                m_stream.close();
            }
        }
        catch (std::ofstream::failure e) {
            // TODO: Create solution
            /* DO NOTHING = We can't to do anything, maybe */
        }
    }

    void FileWriterByParts::setConsumerBuffer(const std::shared_ptr<twPro::LRDataBuffer>& _buffer) noexcept
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        m_buffer = _buffer;
    }

    Result<FILE_WRITER_CODES> FileWriterByParts::work(std::atomic_bool & _stopFlag) noexcept
    {
        std::lock_guard<std::mutex> lock(work_mutex);

        if (!m_buffer) {
            return { FILE_WRITER_CODES::INTERNAL_ERROR, "Internal error: data buffer doesn't exist" };
        }

        while (!_stopFlag.load()) {

            std::shared_ptr<twPro::DataUnit> unit = m_buffer->right_popWait(UNIT_WAIT_TIMEOUT_MS).lock();

            if (!unit) {
                continue;
            }

            try {
                // setup offset in the output file
                size_t offset = unit->id * unit->size;
                m_stream.seekp(offset, std::ios_base::beg);

                // write
                m_stream.write(reinterpret_cast<char*>(unit->ptr), unit->dataSize);
            }
            catch (std::ofstream::failure e) {
                m_buffer->right_push(unit);
                std::string text = e.what();
                return { FILE_WRITER_CODES::WRITING_ERROR, text.empty() ? "Unexpected file writing error" : text };
            }

            ++m_consumedDataBlocks;
            m_consumedDataLength += unit->dataSize;
            m_buffer->left_push(unit);
            currentConsumedDataUnits_notify(m_consumedDataBlocks);
        }

        return FILE_WRITER_CODES::OK;
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
