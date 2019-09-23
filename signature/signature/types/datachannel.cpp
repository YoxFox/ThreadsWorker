#include "datachannel.h"

namespace twPro {

    DataChannel::DataChannel() noexcept
    {
    }

    DataChannel::~DataChannel() noexcept
    {
    }

    void DataChannel::listen(std::atomic_bool & _stopListening, const long long _waitListeningCycleMS)
    {
        // We forbid to have more than one listener at the time
        std::lock_guard<std::mutex> lock(m_mutex);

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