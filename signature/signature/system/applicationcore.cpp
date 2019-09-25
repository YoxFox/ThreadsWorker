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

        /*
        params.blockSize = 1024 * 1024;
        params.source = "C:\\Users\\YoxFox\\Downloads\\alita_film.mkv";
        //params.source = "C:\\Users\\YoxFox\\Downloads\\boost_1_71_0.7z";
        params.destination = "C:\\Users\\YoxFox\\Downloads\\!result.sign";
        params.workerType = twPro::WorkersTypes::MD5_hex;
        */

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

            interactive->pushMessage("The task is successfully done", IInteractive::MessageType::INFO_m);
        });

        ct.join();

        stopInteractive();

        return 0;
    }

}