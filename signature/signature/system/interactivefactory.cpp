#include "interactivefactory.h"

namespace twPro {

    InteractiveFactory::InteractiveFactory() :
        m_interactive(nullptr)
    {
    }

    std::shared_ptr<IInteractive> InteractiveFactory::interactive() const noexcept
    {
        return m_interactive;
    }

    void InteractiveFactory::setInteractive(std::shared_ptr<IInteractive> _interactive)
    {
        m_interactive = _interactive;
    }

}
