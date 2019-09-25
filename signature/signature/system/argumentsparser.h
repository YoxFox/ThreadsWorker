#pragma once

#include "applicationparameters.h"
#include "constructordefines.h"

namespace twPro {

    class ArgumentsParser final
    {
    public:

        COPY_MOVE_FORBIDDEN(ArgumentsParser)

        static twPro::ApplicationParameters parseParameters(int argc, const char *argv[]) noexcept;

    private:

        ArgumentsParser() noexcept;
        ~ArgumentsParser() noexcept;

        static twPro::WorkersTypes workTypeByString(const std::string & _val) noexcept;

    };

}
