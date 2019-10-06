#ifndef __ITASK_H__
#define __ITASK_H__

#include <atomic>
#include "../types/result.h"

namespace twPro {

    enum class TASK_CODES : RESULT_CODES_TYPE
    {
        MAIN_RESULT_CODES,
        INTERNAL_ERROR,

    };

    class ITask
    {
    public:
        virtual ~ITask() noexcept {}

        virtual twPro::Result<twPro::TASK_CODES> run(std::atomic_bool & _stopFlag) noexcept = 0;

    };

}

#endif // __ITASK_H__