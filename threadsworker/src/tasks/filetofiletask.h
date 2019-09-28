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
        virtual void preCheck(const twPro::SourceToResultTemplateTask_params & _params);
        virtual void setupSources(std::shared_ptr<twPro::DataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::DataBuffer> _resultBufferPtr);
        virtual std::function<void()> prepareSourceJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret);
        virtual std::function<void()> prepareResultJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret);
        virtual std::function<void()> prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret);
        virtual void setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret);
        virtual void clear();

    protected:

        virtual size_t totalData() const;
        virtual size_t totalWriteDataUnits() const;

        std::shared_ptr<twPro::FileReaderByParts> m_sourceFilePtr;
        std::shared_ptr<twPro::FileWriterByParts> m_resultFilePtr;

    };

}

#endif // __FILETOFILETASK_H__