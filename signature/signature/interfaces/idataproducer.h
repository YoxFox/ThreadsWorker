#pragma once

namespace twPro {

    class IDataProducer
    {
    public:

        virtual ~IDataProducer() {}

        // Working with producing some data. It takes all thread time.
        virtual void work() = 0;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        virtual void stop() = 0;

        // True if the data producing is done, otherwise false.
        // If true, the work() method do nothing and returns immediately.
        virtual bool isDone() const noexcept = 0;

    };

}