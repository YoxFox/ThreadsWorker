#ifndef __SOURCETORESULTTAMPLATETASK_H__
#define __SOURCETORESULTTAMPLATETASK_H__

#include <string>
#include <memory>
#include <mutex>

#include "../interfaces/itask.h"
#include "../interfaces/iworker.h"
#include "../types/lrdatabuffer.h"
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
        virtual ~SourceToResultTemplateTask() noexcept;

        COPY_FORBIDDEN(SourceToResultTemplateTask)

        inline void setWorker(const std::shared_ptr<twPro::IWorker> & _worker) noexcept
        {
            m_worker = _worker;
        }

        twPro::Result<twPro::TASK_CODES> run(std::atomic_bool & _stopFlag) noexcept override;

    protected:

        // Steps for template
        virtual twPro::Result<twPro::TASK_CODES> preCheck(const twPro::SourceToResultTemplateTask_params & _params) noexcept = 0;
        virtual twPro::Result<twPro::TASK_CODES> setupSources(std::shared_ptr<twPro::LRDataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::LRDataBuffer> _resultBufferPtr) noexcept = 0;
        virtual std::function<void()> prepareSourceJob(std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept = 0;
        virtual std::function<void()> prepareResultJob(std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept = 0;
        virtual std::function<void()> prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept = 0;
        virtual void setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, twPro::Result<twPro::TASK_CODES> & _ret) noexcept = 0;
        virtual void clear() noexcept = 0;

        inline twPro::SourceToResultTemplateTask_params parameters() const noexcept { return m_params; }

    private:

        twPro::RetVal<twPro::TASK_CODES, std::shared_ptr<twPro::LRDataBuffer>> createBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize) const noexcept;

        twPro::SourceToResultTemplateTask_params m_params;
        std::shared_ptr<twPro::IWorker> m_worker;
        std::mutex m_mutex;

        std::shared_ptr<twPro::LRDataBuffer> m_sourceBufferPtr;
        std::shared_ptr<twPro::LRDataBuffer> m_resultBufferPtr;

    };

} 

#endif // __SOURCETORESULTTAMPLATETASK_H__
