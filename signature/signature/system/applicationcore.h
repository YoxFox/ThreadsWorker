#ifndef __APPLICATIONCORE_H__
#define __APPLICATIONCORE_H__

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

#endif // __APPLICATIONCORE_H__