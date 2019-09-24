#pragma once

#include "applicationparameters.h"

namespace twPro {

    class ArgumentsParser final
    {
    public:

        static twPro::ApplicationParameters parseParameters(int argc, const char *argv[]);

    private:

        ArgumentsParser();
        ~ArgumentsParser();

        static twPro::WorkersTypes workTypeByString(const std::string & _val);

    };

}
