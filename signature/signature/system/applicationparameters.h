#pragma once

#include <string>

#include "../types/workerstypes.h"

namespace twPro {

    struct ApplicationParameters final
    {
        twPro::WorkersTypes workerType;
        std::string source;
        std::string destination;
        size_t blockSize;

        ApplicationParameters() : blockSize(0) {}
    };

}
