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
        FileToFileByBlocksTask(const FileToFileByBlocksTask_params & _params) noexcept;
        ~FileToFileByBlocksTask() noexcept;

        inline void setWorker(const std::shared_ptr<IWorker> & _worker) noexcept
        {
            m_worker = _worker;
        }

        void run(std::atomic_bool & _stopFlag) override;

    private:

        FileToFileByBlocksTask_params m_params;
        std::shared_ptr<IWorker> m_worker;
        std::mutex m_mutex;

        std::shared_ptr<twPro::FileReaderByParts> m_sourceFilePtr;
        std::shared_ptr<twPro::FileWriterByParts> m_resultFilePtr;
        std::shared_ptr<twPro::DataBuffer> m_sourceBufferPtr;
        std::shared_ptr<twPro::DataBuffer> m_resultBufferPtr;

    };

}
