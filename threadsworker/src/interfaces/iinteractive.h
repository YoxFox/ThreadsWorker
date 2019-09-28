#ifndef __IINTERACTIVE_H__
#define __IINTERACTIVE_H__

#include <memory>
#include <string>

#include "../types/datachannel.h"

namespace twPro {

    class IInteractive
    {
    public:

        virtual ~IInteractive() noexcept {}

        struct Progress {
            std::string title;
            size_t current;
            size_t total;

            inline operator bool() const noexcept { return total != 0; }

            Progress() : title(), current(0), total(0) {}
            Progress(const std::string & _title, const size_t & _current, const size_t & _total) : title(_title), current(_current), total(_total) {}
        };

        virtual std::shared_ptr<twPro::Notifier<twPro::IInteractive::Progress>> createProgressBar() noexcept = 0;

        enum class MessageType : unsigned int
        {
            INFO_m = 0,
            WARNING_m,
            ERROR_m
        };

        virtual void pushMessage(const std::string _message, const twPro::IInteractive::MessageType & _type = twPro::IInteractive::MessageType::INFO_m) noexcept = 0;
    };

}

#endif // __IINTERACTIVE_H__