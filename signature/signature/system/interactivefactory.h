#pragma once

// This class is the factory for injection of the Interactive dependency
// In the future, we have to develop more flexible solution for DI

#include <memory>
#include <sstream>
#include "../interfaces/iinteractive.h"

namespace twPro {

    class InteractiveFactory final
    {
    public:

        static twPro::InteractiveFactory * instance() noexcept {
            static twPro::InteractiveFactory f;
            return &f;
        }

        std::shared_ptr<twPro::IInteractive> interactive() const noexcept;
        void setInteractive(std::shared_ptr<twPro::IInteractive> _interactive) noexcept;

    private:

        InteractiveFactory() noexcept;

        std::shared_ptr<twPro::IInteractive> m_interactive;

    };

    inline std::shared_ptr<twPro::IInteractive> interactive() noexcept { return twPro::InteractiveFactory::instance()->interactive(); }

}

#include <sstream>

#define B_INFO { std::ostringstream s; s
#define E_INFO ' '; interactive()->pushMessage(s.str()); }
