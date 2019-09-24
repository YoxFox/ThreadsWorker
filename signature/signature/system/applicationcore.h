#pragma once

namespace twPro {

    struct ApplicationParameters;
    class ApplicationCore final
    {
    public:

        ApplicationCore() noexcept;
        ~ApplicationCore() noexcept;

        // The main function that starts all work
        int exec(int argc, const char *argv[]) noexcept;

    };

}
