#ifndef __FILETOFILETASK_H__
#define __FILETOFILETASK_H__

#include "sourcetoresulttemplatetask.h"

#include "../services/filereaderbyparts.h"
#include "../services/filewriterbyparts.h"

namespace twPro {

    class FileToFileTask : public twPro::SourceToResultTemplateTask
    {
    public:

        FileToFileTask(const twPro::SourceToResultTemplateTask_params & _params) noexcept;
        virtual ~FileToFileTask() noexcept;

        COPY_FORBIDDEN(FileToFileTask)

        // Steps for template
        virtual twPro::Result<twPro::TASK_CODES> preCheck(const twPro::SourceToResultTemplateTask_params & _params) noexcept;
        virtual twPro::Result<twPro::TASK_CODES> setupSources(std::shared_ptr<twPro::LRDataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::LRDataBuffer> _resultBufferPtr) noexcept;
        virtual std::function<void()> prepareSourceJob(std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept;
        virtual std::function<void()> prepareResultJob(std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept;
        virtual std::function<void()> prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept;
        virtual void setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept;
        virtual void clear() noexcept;

    protected:

        twPro::RetVal<twPro::TASK_CODES, std::shared_ptr<twPro::FileReaderByParts>> createFileReader(const std::string & _filePath) const noexcept;
        twPro::RetVal<twPro::TASK_CODES, std::shared_ptr<twPro::FileWriterByParts>> createFileWriter(const std::string & _filePath) const noexcept;

        virtual size_t totalData() const noexcept;
        virtual size_t totalWriteDataUnits() const noexcept;

        std::shared_ptr<twPro::FileReaderByParts> m_sourceFilePtr;
        std::shared_ptr<twPro::FileWriterByParts> m_resultFilePtr;

    };

}

#endif // __FILETOFILETASK_H__