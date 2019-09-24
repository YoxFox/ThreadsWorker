#pragma once

#include "applicationparameters.h"

namespace twPro {

    class ArgumentsParser final
    {
    public:

        static twPro::ApplicationParameters parseParameters(int argc, const char *argv[]) noexcept;

    private:

        ArgumentsParser() noexcept;
        ~ArgumentsParser() noexcept;

        static twPro::WorkersTypes workTypeByString(const std::string & _val) noexcept;

    };

}
