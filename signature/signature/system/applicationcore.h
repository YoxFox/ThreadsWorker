#pragma once

namespace twPro {

    class ApplicationCore
    {
    public:

        ApplicationCore();
        ~ApplicationCore();

        // The main function that starts all work
        int exec(int argc, const char *argv[]) noexcept;

    };

}
