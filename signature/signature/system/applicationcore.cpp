#include "applicationcore.h"

#include <thread>

#include "../tasks/taskmanager.h"
#include "../services/consoleinteractive.h"
#include "../system/interactivefactory.h"
#include "argumentsparser.h"

namespace twPro {

    ApplicationCore::ApplicationCore() noexcept
    {
    }

    ApplicationCore::~ApplicationCore() noexcept
    {
    }

    int ApplicationCore::exec(int argc, const char *argv[]) noexcept
    {
        // === SETUP INTERACTIVE ===

        std::shared_ptr<twPro::ConsoleInteractive> interactive(new twPro::ConsoleInteractive());
        twPro::InteractiveFactory::instance()->setInteractive(interactive);

        std::atomic_bool stopInteractiveAtom = false;
        std::thread interactiveThread([&stopInteractiveAtom, interactive]() {
            interactive->run(stopInteractiveAtom);
        });

        auto stopInteractive = [&stopInteractiveAtom, &interactiveThread]() {
            // Just sleep for interactive if it has some view events
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            stopInteractiveAtom = true;
            interactiveThread.join();
        };

        // === TASK ===

        twPro::ApplicationParameters params = twPro::ArgumentsParser::parseParameters(argc, argv);

        if (params.workerType == WorkersTypes::Unknown) {
            interactive->pushMessage("Incorrect (unknown) task", IInteractive::MessageType::ERROR_m);
            stopInteractive();
            return 0;
        }

        twPro::TaskManager manager;

        std::shared_ptr<twPro::ITask> task = manager.createTask(params);

        if (!task) {
            interactive->pushMessage("Incorrect task", IInteractive::MessageType::ERROR_m);
            stopInteractive();
            return 0;
        }

        std::atomic_bool stopFlag = false;

        std::thread ct([&task, &stopFlag, &interactive]() {
            try {
                task->run(stopFlag);
            }
            catch (const std::exception& ex) {
                interactive->pushMessage(ex.what(), IInteractive::MessageType::ERROR_m);
                return;
            }
            catch (const std::string& ex) {
                interactive->pushMessage(ex, IInteractive::MessageType::ERROR_m);
                return;
            }
            catch (...) {
                interactive->pushMessage("The task returns unknown error", IInteractive::MessageType::ERROR_m);
                return;
            }

            // TODO: May be it's not good to show it if the task is not fully completed by some internal errors
            interactive->pushMessage("The task is successfully done", IInteractive::MessageType::INFO_m);
        });

        ct.join();

        stopInteractive();

        return 0;
    }

}