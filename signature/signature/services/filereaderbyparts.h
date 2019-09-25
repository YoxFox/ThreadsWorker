#ifndef __FILEREADERBYPARTS_H__
#define __FILEREADERBYPARTS_H__

#include <memory>
#include <fstream>
#include <atomic>

#include "../system/constructordefines.h"
#include "../types/databuffer.h"
#include "../types/datachannel.h"

// IMPORTANT: This class is single thread worker.
//            If some thread calls work() method, other thread will wait the end of the first thread by mutex inside work() method.

namespace twPro {

    class FileReaderByParts final
    {
    public:

        FileReaderByParts(const std::string & _filePath);
        ~FileReaderByParts() noexcept;

        COPY_FORBIDDEN(FileReaderByParts)

        // Output data buffer for producing
        void setProducerBuffer(const std::shared_ptr<twPro::DataBuffer> & _buffer) noexcept;

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag or by some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        void work(std::atomic_bool & _stopFlag);

        // True if the data producing is done, otherwise false.
        // If true, the work() method do nothing and returns immediately.
        bool isDone() const noexcept;

        size_t currentProducedData() const noexcept;
        size_t currentProducedDataUnits() const noexcept;
        size_t totalData() const noexcept; // It returns max(return_type) for infinity data

        NOTIFIER_MEMBER(currentProducedDataUnits, size_t)

    private:

        std::atomic_bool m_isDone;
        std::atomic_bool m_isStopped;

        std::atomic<size_t> m_idPart;
        std::atomic<size_t> m_producedDataLength;
        std::atomic<size_t> m_fileLength;

        std::mutex work_mutex;

        std::shared_ptr<twPro::DataBuffer> m_buffer;
        std::ifstream m_stream;

    };

}

#endif // __FILEREADERBYPARTS_H__