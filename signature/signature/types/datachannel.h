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
        virtual void processNotifications() noexcept = 0;
        virtual bool isEmpty() noexcept = 0;

    };

    template <class T>
    class Notifier final : public INotifier
    {
    public:

        Notifier() {}

        void notify(const T & _val) noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_notifyQueue.push(_val);
        }

        void setCallBack(std::function<void(const T &)> _func)
        {
            m_func = _func;
        }

    private:

        void processNotifications() noexcept override
        {
            m_func(popNotifyValue());
        }

        bool isEmpty() noexcept override
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_notifyQueue.empty();
        }

        T popNotifyValue()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_notifyQueue.empty()) {
                return T();
            }

            T val = m_notifyQueue.front();
            m_notifyQueue.pop();
            return val;
        }

        std::queue<T> m_notifyQueue;
        std::mutex m_mutex;
        std::function<void(const T &)> m_func = [](const T &){};

    };

    class DataChannel final
    {
    public:
        DataChannel();
        ~DataChannel();

        template<class T>
        std::shared_ptr<Notifier<T>> createNotifier()
        {
            std::shared_ptr<Notifier<T>> sPtr(new Notifier<T>());
            m_notifiers.push_back(sPtr);
            return sPtr;
        }

        void listen(std::atomic_bool & _stopListening, const long long _waitListeningCycleMS = 10);

    private:

        std::vector<std::shared_ptr<INotifier>> m_notifiers;

    };

}