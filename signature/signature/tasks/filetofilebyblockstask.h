#pragma once

#include <string>
#include <memory>
#include <mutex>

#include "../services/filereaderbyparts.h"
#include "../services/filewriterbyparts.h"
#include "../interfaces/itask.h"
#include "../interfaces/iworker.h"

namespace twPro {

    struct FileToFileByBlocksTask_params final
    {
        std::string sourceFilePath;
        std::string resultFilePath;
        size_t blockSize;
    };

    class FileToFileByBlocksTask final : public twPro::ITask
    {
    public:
        FileToFileByBlocksTask(const twPro::FileToFileByBlocksTask_params & _params) noexcept;
        ~FileToFileByBlocksTask() noexcept;

        inline void setWorker(const std::shared_ptr<twPro::IWorker> & _worker) noexcept
        {
            m_worker = _worker;
        }

        void run(std::atomic_bool & _stopFlag) override;

    private:

        // Replace it by common result object
        struct _pResult
        {
            std::string error;
            bool isError;

            inline operator bool() const { return !isError; }

            _pResult(const std::string & _message) : error(_message), isError(true) {}
            _pResult() : isError(false) {}
            _pResult(const bool _ret) : isError(!_ret) {}
        };

        twPro::FileToFileByBlocksTask_params m_params;
        std::shared_ptr<twPro::IWorker> m_worker;
        std::mutex m_mutex;

        std::shared_ptr<twPro::FileReaderByParts> m_sourceFilePtr;
        std::shared_ptr<twPro::FileWriterByParts> m_resultFilePtr;
        std::shared_ptr<twPro::DataBuffer> m_sourceBufferPtr;
        std::shared_ptr<twPro::DataBuffer> m_resultBufferPtr;

    };

}
