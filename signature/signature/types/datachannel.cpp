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
            std::lock_guard<std::mutex> lock(m_new_notifier_mutex);
            for (auto notify : m_notifiers) {
                if (!notify->isEmpty()) {
                    notify->processNotifications();
                }
            }
        }
    }

    void DataChannel::listen(std::function<bool()> _stopCondition, const long long _waitListeningCycleMS)
    {
        // We forbid to have more than one listener at the time
        std::lock_guard<std::mutex> lock(m_mutex);

        while (!_stopCondition()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(_waitListeningCycleMS));
            std::lock_guard<std::mutex> lock(m_new_notifier_mutex);
            for (auto notify : m_notifiers) {
                if (!notify->isEmpty()) {
                    notify->processNotifications();
                }
            }
        }
    }

}
