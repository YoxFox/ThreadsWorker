#pragma once

#include <memory>
#include <fstream>
#include <atomic>

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
        void work() override;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        void stop() noexcept override;

    private:

        std::atomic_bool m_isStopped;

        std::mutex work_mutex;

        std::shared_ptr<twPro::DataBuffer> m_buffer;
        std::ofstream m_stream;

    };

}

