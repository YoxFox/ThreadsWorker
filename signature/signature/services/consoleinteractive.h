#pragma once

// Platformm dependency!
// TODO: move it to the platform dependency section
#include <Windows.h>

#include <atomic>
#include <mutex>

#include "../types/datachannel.h"
#include  "../interfaces/iinteractive.h"

#ifndef FOREGROUND_WHITE
#define FOREGROUND_WHITE 15
#endif // FOREGROUND_WHITE

namespace twPro {

    class ConsoleInteractive final : public twPro::IInteractive
    {
    public:

        ConsoleInteractive() noexcept;
        ~ConsoleInteractive() noexcept;

        // Takes all thread time, it runs interactive loop
        void run(std::atomic_bool & _stopFlag);

        std::shared_ptr<twPro::Notifier<twPro::IInteractive::Progress>> createProgressBar() noexcept override;
        void pushMessage(const std::string _message, const twPro::IInteractive::MessageType & _type = twPro::IInteractive::MessageType::INFO_m) noexcept override;

    private:

        struct ColorChanger
        {
            // It returns standart (white) color after destruction
            ColorChanger(const int _color = FOREGROUND_WHITE) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _color); }
            ~ColorChanger() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE); }
        };

        void showProgress(const twPro::IInteractive::Progress & _progress) noexcept;
        void moveProgressBarDown() noexcept;
        SHORT consoleWidth() const;

        twPro::DataChannel m_dataChannel;
        std::mutex m_consoleMutex;

        twPro::IInteractive::Progress m_currentProgress;

    };

}
