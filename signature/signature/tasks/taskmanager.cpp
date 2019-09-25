#include "taskmanager.h"

#include "../tasks/filetofiletask.h"

#include "../services/workers/md5hashworker.h"

namespace twPro {

    TaskManager::TaskManager() noexcept
    {
    }

    TaskManager::~TaskManager() noexcept
    {
    }

    std::shared_ptr<twPro::ITask> TaskManager::createTask(const ApplicationParameters & _params) const noexcept
    {
        switch (_params.workerType)
        {
        case WorkersTypes::Unknown: return nullptr;
        case WorkersTypes::MD5_hex:
        case WorkersTypes::MD5_byte: return createMD5Task(_params);
        }

        return nullptr;
    }

    std::shared_ptr<twPro::ITask> TaskManager::createMD5Task(const ApplicationParameters & _params) const noexcept
    {

        twPro::SourceToResultTemplateTask_params taskParams;

        taskParams.blockSize = _params.blockSize;
        taskParams.source = _params.source;
        taskParams.result = _params.destination;

        std::shared_ptr<twPro::IWorker> worker(nullptr);

        switch (_params.workerType)
        {
        case WorkersTypes::MD5_hex: worker.reset(new MD5HashWorker()); break;
        case WorkersTypes::MD5_byte: /* DO NOTHING | TODO */; break;
        default: /* DO NOTHING */ break;
        }

        std::shared_ptr<twPro::FileToFileTask> task(nullptr);

        if (worker) {
            task.reset(new FileToFileTask(taskParams));
            task->setWorker(worker);
        }

        return task;
    }

}
