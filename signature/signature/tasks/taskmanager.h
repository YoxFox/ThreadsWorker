#ifndef __TASKMANAGER_H__
#define __TASKMANAGER_H__

#include <memory>

#include "../interfaces/itask.h"
#include "../system/applicationparameters.h"

namespace twPro {

    class TaskManager final
    {
    public:
        TaskManager() noexcept;
        ~TaskManager() noexcept;

        std::shared_ptr<twPro::ITask> createTask(const twPro::ApplicationParameters & _params) const noexcept;

    private:

        std::shared_ptr<twPro::ITask> createMD5Task(const twPro::ApplicationParameters & _params) const noexcept;

    };

}

#endif // __TASKMANAGER_H__