#include "interactivefactory.h"

namespace twPro {

    InteractiveFactory::InteractiveFactory() noexcept :
        m_interactive(nullptr)
    {
    }

    std::shared_ptr<IInteractive> InteractiveFactory::interactive() const noexcept
    {
        return m_interactive;
    }

    void InteractiveFactory::setInteractive(std::shared_ptr<IInteractive> _interactive) noexcept
    {
        m_interactive = _interactive;
    }

}
