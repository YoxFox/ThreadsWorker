#pragma once

#include <string>
#include <memory>
#include <mutex>

#include "../interfaces/itask.h"
#include "../interfaces/iworker.h"
#include "../types/databuffer.h"
#include "../types/datachannel.h"

namespace twPro {

    struct SourceToResultTemplateTask_params final
    {
        std::string source;
        std::string result;
        size_t blockSize;
    };

    class SourceToResultTemplateTask : public twPro::ITask
    {
    public:
        SourceToResultTemplateTask(const twPro::SourceToResultTemplateTask_params & _params) noexcept;
        ~SourceToResultTemplateTask() noexcept;

        inline void setWorker(const std::shared_ptr<twPro::IWorker> & _worker) noexcept
        {
            m_worker = _worker;
        }

        void run(std::atomic_bool & _stopFlag) override;

    protected:

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

        // Steps for template
        virtual void preCheck(const twPro::SourceToResultTemplateTask_params & _params) = 0;
        virtual void setupSources(std::shared_ptr<twPro::DataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::DataBuffer> _resultBufferPtr) = 0;
        virtual std::function<void()> prepareSourceJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret) = 0;
        virtual std::function<void()> prepareResultJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret) = 0;
        virtual std::function<void()> prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret) = 0;
        virtual void setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret) = 0;
        virtual void clear() = 0;

        inline twPro::SourceToResultTemplateTask_params parameters() const { return m_params; }

    private:

        twPro::SourceToResultTemplateTask_params m_params;
        std::shared_ptr<twPro::IWorker> m_worker;
        std::mutex m_mutex;

        std::shared_ptr<twPro::DataBuffer> m_sourceBufferPtr;
        std::shared_ptr<twPro::DataBuffer> m_resultBufferPtr;

    };

}
