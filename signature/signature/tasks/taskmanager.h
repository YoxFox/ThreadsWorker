#pragma once

#include <memory>

#include "../interfaces/itask.h"
#include "../system/applicationparameters.h"

namespace twPro {

    class TaskManager final
    {
    public:
        TaskManager() noexcept;
        ~TaskManager() noexcept;

        std::shared_ptr<twPro::ITask> createTask(const ApplicationParameters & _params) const noexcept;

    private:

        std::shared_ptr<twPro::ITask> createMD5Task(const ApplicationParameters & _params) const noexcept;

    };

}
