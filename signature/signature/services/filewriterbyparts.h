#pragma once

#include <memory>
#include <fstream>
#include <atomic>
#include <vector>

#include "../interfaces/idataconsumer.h"
#include "../types/databuffer.h"

// IMPORTANT: This class is single thread worker.
//            If some thread calls work() method, other thread will wait the end of the first thread by mutex inside work() method.

namespace twPro {

    class FileWriterByParts final : public twPro::IDataConsumer
    {
    public:

        FileWriterByParts(const std::string & _filePath, const std::shared_ptr<twPro::DataBuffer> & _buffer);
        ~FileWriterByParts() override;

        // Working with producing some data. It takes all thread time.
        void work(std::atomic_bool & _stopFlag) override;

        unsigned long long currentConsumedData() const noexcept override;

        EVENT_HANDLER_MEMBER(currentConsumedData, unsigned long long)

    private:

        std::mutex work_mutex;

        std::atomic_ullong m_consumedDataLength;
        std::shared_ptr<twPro::DataBuffer> m_buffer;
        std::ofstream m_stream;

    };

}

