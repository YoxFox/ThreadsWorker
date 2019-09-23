#pragma once

#include <string>

#include "workerstypes.h"

namespace twPro {

    struct ApplicationParameters final
    {
        twPro::WorkersTypes workerType;
        std::string source;
        std::string destination;
        int blockSize;
    };

}
