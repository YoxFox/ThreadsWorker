#pragma once

namespace twPro {

    class IWorker
    {
    public:

        virtual ~IWorker() {}

        // It takes all thread time.
        virtual void work() = 0;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        virtual void stop() noexcept = 0;

    };

}