#include "applicationcore.h"

#include <thread>

#include "../tasks/taskmanager.h"
#include "argumentsparser.h"

#include <mutex>
#include <iostream>
static std::mutex log_mutex;
#define BLOG { std::lock_guard<std::mutex> lock(log_mutex); std::cout 
#define ELOG "\n";}

namespace twPro {

    ApplicationCore::ApplicationCore()
    {
    }

    ApplicationCore::~ApplicationCore()
    {
    }

    int ApplicationCore::exec(int argc, const char *argv[]) noexcept
    {
        twPro::ApplicationParameters params = twPro::ArgumentsParser::parseParameters(argc, argv);

        params.blockSize = 1024 * 1024;
        params.source = "C:\\Users\\YoxFox\\Downloads\\alita_film.mkv";
        params.destination = "C:\\Users\\YoxFox\\Downloads\\!result.sign";
        params.workerType = twPro::WorkersTypes::MD5_hex;

        twPro::TaskManager manager;

        std::shared_ptr<twPro::ITask> task = manager.createTask(params);

        if (!task) {
            BLOG << "Incorrect task" << ELOG;
            return 1;
        }

        std::atomic_bool stopFlag = false;

        std::thread ct([&task, &stopFlag]() {
            task->run(stopFlag);
        });

        ct.join();

        system("pause");
        return 0;
    }

}