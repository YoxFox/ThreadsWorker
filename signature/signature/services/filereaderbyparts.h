#pragma once

#include <memory>
#include <fstream>
#include <atomic>

#include "../interfaces/idataproducer.h"
#include "../types/databuffer.h"

// IMPORTANT: This class is single thread worker.
//            If some thread calls work() method, other thread will wait the end of the first thread by mutex inside work() method.

namespace twPro {

    class FileReaderByParts final : public twPro::IDataProducer
    {
    public:

        FileReaderByParts(const std::string & _filePath, const std::shared_ptr<twPro::DataBuffer> & _buffer);
        ~FileReaderByParts() override;

        // Working with producing some data. It takes all thread time.
        void work() override;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        void stop() noexcept override;

        // True if the data producing is done, otherwise false.
        // If true, the work() method do nothing and returns immediately.
        bool isDone() const noexcept override;

    private:

        std::atomic_bool m_isDone;
        std::atomic_bool m_isStopped;
        unsigned __int64 m_idPart;
        unsigned __int64 m_fileLength;

        std::mutex work_mutex;

        std::shared_ptr<twPro::DataBuffer> m_buffer;
        std::ifstream m_stream;

    };

}
