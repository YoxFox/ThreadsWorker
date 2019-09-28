#ifndef __ITASK_H__
#define __ITASK_H__

#include <atomic>

namespace twPro {

    class ITask
    {
    public:
        virtual ~ITask() noexcept {}

        virtual void run(std::atomic_bool & _stopFlag) = 0;

    };

}

#endif // __ITASK_H__