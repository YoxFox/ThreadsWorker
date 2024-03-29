#ifndef __FILEWRITERBYPARTS_H__
#define __FILEWRITERBYPARTS_H__

#include <memory>
#include <fstream>
#include <atomic>
#include <vector>

#include "../system/constructordefines.h"
#include "../types/lrdatabuffer.h"
#include "../types/datachannel.h"
#include "../types/result.h"

// IMPORTANT: This class is single thread worker.
//            If some thread calls work() method, other thread will wait the end of the first thread by mutex inside work() method.

namespace twPro {

    enum class FILE_WRITER_CODES : RESULT_CODES_TYPE
    {
        MAIN_RESULT_CODES,
        WRITING_ERROR,
        INTERNAL_ERROR
    };

    class FileWriterByParts final
    {
    public:

        FileWriterByParts(const std::string & _filePath);
        ~FileWriterByParts() noexcept;

        COPY_FORBIDDEN(FileWriterByParts)

        // Output data buffer for producing
        void setConsumerBuffer(const std::shared_ptr<twPro::LRDataBuffer> & _buffer) noexcept;

        // Working with producing some data. It takes all thread time.
        twPro::Result<twPro::FILE_WRITER_CODES> work(std::atomic_bool & _stopFlag) noexcept;

        size_t currentConsumedData() const noexcept;
        size_t currentConsumedDataUnits() const noexcept;

        NOTIFIER_MEMBER(currentConsumedDataUnits, size_t)

    private:

        std::mutex work_mutex;

        std::atomic<size_t> m_consumedDataBlocks;
        std::atomic<size_t> m_consumedDataLength;
        std::shared_ptr<twPro::LRDataBuffer> m_buffer;
        std::ofstream m_stream;

    };

}

#endif // __FILEWRITERBYPARTS_H__