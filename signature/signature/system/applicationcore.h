#pragma once

namespace twPro {

    class ApplicationCore final
    {
    public:

        ApplicationCore() noexcept;
        ~ApplicationCore() noexcept;

        // The main function that starts all work
        int exec(int argc, const char *argv[]) noexcept;

    };

}
