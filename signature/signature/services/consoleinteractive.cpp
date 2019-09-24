#include "consoleinteractive.h"

#include <iostream>

#ifndef FOREGROUND_YELLOW
#define FOREGROUND_YELLOW 14
#endif // FOREGROUND_YELLOW

namespace twPro {

    static const size_t progressBarWidth = 70;

    void ShowConsoleCursor(bool showFlag)
    {
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_CURSOR_INFO     cursorInfo;

        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = showFlag; // set the cursor visibility
        SetConsoleCursorInfo(out, &cursorInfo);
    }

    void ConsoleInteractive::showProgress(const twPro::IInteractive::Progress & _progress) noexcept
    {
        ShowConsoleCursor(false);
        ColorChanger ch(FOREGROUND_GREEN); (void)ch; /* UNUSED */

        // Overwrite protection: it has incorrect behaviour for narrow console
        size_t minConsoleWidth = _progress.title.length() + progressBarWidth + /* service */ 10;
        if ((size_t)consoleWidth() < minConsoleWidth) {
            return;
        }

        //minus label len
        int scaleWidth = progressBarWidth - _progress.title.length();
        size_t scalePosition = (_progress.current * scaleWidth) / _progress.total;

        size_t percent = (_progress.current * 100) / _progress.total;

        /* Title */             printf(" %s [", _progress.title);
        /* Scale filling */     for (size_t i = 0; i < scalePosition; i++)  printf("%c", '=');
        /* Space filling */     printf("% *c", scaleWidth - scalePosition + 1, ']');
        
        printf(" %3d%%\r", percent);
    }

    void ConsoleInteractive::moveProgressBarDown() noexcept
    {
        printf("%c[2K", 27); // clear current string
        printf("\n");
        showProgress(m_currentProgress);
        printf("\033[F"); // up cursor to the begin of the top string
    }

    SHORT ConsoleInteractive::consoleWidth() const
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        SHORT columns;

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

        return columns;
    }

    ConsoleInteractive::ConsoleInteractive() noexcept
    {
        ShowConsoleCursor(false);
        ColorChanger ch(); (void)ch; /* UNUSED */
    }

    ConsoleInteractive::~ConsoleInteractive() noexcept
    {
        ShowConsoleCursor(true);
    }

    void ConsoleInteractive::run(std::atomic_bool & _stopFlag)
    {
        m_dataChannel.listen(_stopFlag);
    }

    std::shared_ptr<twPro::Notifier<twPro::IInteractive::Progress>> ConsoleInteractive::createProgressBar() noexcept
    {
        auto notifier = m_dataChannel.createNotifier<twPro::IInteractive::Progress>(1);

        notifier->setCallBack([this](const twPro::IInteractive::Progress & _progress) {
            std::lock_guard<std::mutex> lock(m_consoleMutex);
            m_currentProgress = _progress;
            showProgress(_progress);
        });

        return notifier;
    }

    void ConsoleInteractive::pushMessage(const std::string _message, const MessageType & _type) noexcept
    {
        std::lock_guard<std::mutex> lock(m_consoleMutex);
        ShowConsoleCursor(false);

        if (m_currentProgress) {
            moveProgressBarDown();
        }

        std::shared_ptr<ColorChanger> ch;
        switch (_type)
        {
        case MessageType::INFO_m: ch.reset(new ColorChanger(FOREGROUND_WHITE)); break;
        case MessageType::WARNING_m: ch.reset(new ColorChanger(FOREGROUND_YELLOW)); break;
        case MessageType::ERROR_m: ch.reset(new ColorChanger(FOREGROUND_RED)); break;
        default: break;
        }

        std::cout << _message << '\n';
    }

}
