#pragma once

// This class is the factory for injection of the Interactive dependency
// In the future, we have to develop more flexible solution for DI

#include <memory>
#include "../interfaces/iinteractive.h"

namespace twPro {

    class InteractiveFactory final
    {
    public:

        static InteractiveFactory * instance() {
            static InteractiveFactory f;
            return &f;
        }

        std::shared_ptr<IInteractive> interactive() const noexcept;
        void setInteractive(std::shared_ptr<IInteractive> _interactive);

    private:

        InteractiveFactory();

        std::shared_ptr<IInteractive> m_interactive;

    };

    inline std::shared_ptr<IInteractive> interactive() { return InteractiveFactory::instance()->interactive(); }

}

#include <sstream>

#define B_INFO { std::ostringstream s; s
#define E_INFO ' '; interactive()->pushMessage(s.str()); }
