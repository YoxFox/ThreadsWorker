#include "datachannel.h"

namespace twPro {

    DataChannel::DataChannel()
    {
    }

    DataChannel::~DataChannel()
    {
    }

    void DataChannel::listen(std::atomic_bool & _stopListening, const long long _waitListeningCycleMS)
    {
        while (!_stopListening) {
            std::this_thread::sleep_for(std::chrono::milliseconds(_waitListeningCycleMS));
            for (auto notify : m_notifiers) {
                if (!notify->isEmpty()) {
                    notify->processNotifications();
                }
            }
        }
    }

}