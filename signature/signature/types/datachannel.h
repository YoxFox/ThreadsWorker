#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

namespace twPro {

    class INotifier {
    public:
        virtual ~INotifier() noexcept {} 
    private:
        
        friend class DataChannel;
        virtual void processNotifications() = 0;
        virtual bool isEmpty() noexcept = 0;

    };

    template <class T>
    class Notifier final : public INotifier
    {
    public:

        Notifier(const size_t _maxQueueItems) noexcept : m_maxQueueItems(_maxQueueItems) {}

        void notify(const T & _val) noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while (m_notifyQueue.size() >= m_maxQueueItems) {
                m_notifyQueue.pop();
            }
            m_notifyQueue.push(_val);
        }

        void setCallBack(std::function<void(const T &)> _func) noexcept
        {
            m_func = _func;
        }

    private:

        void processNotifications() override
        {
            m_func(popNotifyValue());
        }

        bool isEmpty() noexcept override
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_notifyQueue.empty();
        }

        T popNotifyValue() noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_notifyQueue.empty()) {
                return T();
            }

            T val = m_notifyQueue.front();
            m_notifyQueue.pop();
            return val;
        }

        size_t m_maxQueueItems;
        std::queue<T> m_notifyQueue;
        std::mutex m_mutex;
        std::function<void(const T &)> m_func = [](const T &){};

    };

    class DataChannel final
    {
    public:
        DataChannel() noexcept;
        ~DataChannel() noexcept;

        // We can't use std::numeric_limits<size_t>::max() as default value because some system file defines "max" as macro

        template<class T>
        std::shared_ptr<Notifier<T>> createNotifier(const size_t _maxQueueItems = ULLONG_MAX) noexcept
        {
            std::lock_guard<std::mutex> lock(m_new_notifier_mutex);
            std::shared_ptr<Notifier<T>> sPtr(new Notifier<T>(_maxQueueItems));
            m_notifiers.push_back(sPtr);
            return sPtr;
        }

        void listen(std::atomic_bool & _stopListening, const long long _waitListeningCycleMS = 10);
        void listen(std::function<bool()> _stopCondition, const long long _waitListeningCycleMS = 10);

    private:

        std::vector<std::shared_ptr<INotifier>> m_notifiers;
        std::mutex m_mutex;
        std::mutex m_new_notifier_mutex;

    };

}

#define I_NOTIFIER_MEMBER(name, T) \
    virtual void setNotifier_ ## name ## (std::shared_ptr<Notifier<T>> & _notifier) noexcept = 0; \
    virtual void removeNotifier_ ## name ## (std::shared_ptr<Notifier<T>> & _notifier) noexcept = 0;\

#define NOTIFIER_MEMBER(name, T) \
public: \
    void setNotifier_ ## name ## (std::shared_ptr<Notifier<T>> & _notifier) noexcept \
    { \
        m_notifiers_ ## name ## .push_back(_notifier); \
    } \
    void removeNotifier_ ## name ## (std::shared_ptr<Notifier<T>> & _notifier) noexcept \
    { \
        auto new_end = std::remove_if(m_notifiers_ ## name.begin(), m_notifiers_ ## name.end(), [&_notifier](const std::shared_ptr<Notifier<T>> & _n) { return _n.get() == _notifier.get(); }); \
        m_notifiers_ ## name.erase(new_end, m_notifiers_ ## name.end()); \
    } \
private: \
    void name ## _notify(const T & _val) \
    { \
        for ( auto notifier : m_notifiers_ ## name ) { \
            notifier->notify(_val); \
        } \
    } \
    std::vector<std::shared_ptr<Notifier<T>>> m_notifiers_ ## name; \
public:
