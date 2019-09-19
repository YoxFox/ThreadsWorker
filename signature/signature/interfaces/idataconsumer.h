#pragma once

namespace twPro {

    class IDataConsumer
    {
    public:

        virtual ~IDataConsumer() {}

        // Working with consuming some data. It takes all thread time.
        virtual void work() = 0;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        virtual void stop() = 0;

    };

}