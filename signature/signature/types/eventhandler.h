#pragma once

#include <queue>
#include <condition_variable>
#include <functional>
#include <mutex>

template <class T>
struct HEvent
{
    T value;

    HEvent() {}
    HEvent(const T & _val) : value(_val) {}
};

template <class T>
class EventHandler final
{
public:
    EventHandler(const size_t _eventQueueSize = 10) noexcept : m_eventQueueSize(_eventQueueSize) {}
    ~EventHandler() noexcept {}

    size_t currentQueueSize() const 
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return m_eventQueue.size();
    }

    void notify(const HEvent<T> & _event) noexcept
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);

        while (m_eventQueue.size() >= m_eventQueueSize) {
            m_eventQueue.pop();
        }

        m_eventQueue.push(_event);
        m_cv.notify_all();
    }

    void listen(std::function<bool(const HEvent<T> &)> _conditionFunction) noexcept
    {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            if (isQueueEmpty()) {
                std::unique_lock<std::mutex> lk(m_cv_mutex);
                m_cv.wait(lk, [this]() { return !isQueueEmpty(); });
            }

            HEvent<T> hEvent;

            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);

                if (m_eventQueue.empty()) {
                    continue;
                }

                hEvent = popEvent();
            }

            if (_conditionFunction(hEvent)) { return; }
        }
    }

private:

    HEvent<T> popEvent() noexcept
    {
        HEvent<T> hEvent = m_eventQueue.front();
        m_eventQueue.pop();
        return hEvent;
    }

    bool isQueueEmpty()
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return m_eventQueue.empty();
    }

    std::queue<HEvent<T>> m_eventQueue;
    std::condition_variable m_cv;

    mutable std::mutex m_queue_mutex;
    std::mutex m_cv_mutex;

    size_t m_eventQueueSize;

};

#define I_EVENT_HANDLER_MEMBER(name, T) \
    virtual void setEventHandler_ ## name ## (std::shared_ptr<EventHandler<T>> & _eventHandler) noexcept = 0; \

#define EVENT_HANDLER_MEMBER(name, T) \
public: \
    void setEventHandler_ ## name ## (std::shared_ptr<EventHandler<T>> & _eventHandler) noexcept \
    { \
        m_eventHandlers_ ## name ## .push_back(_eventHandler); \
    } \
private: \
    void eventHandler_ ## name ## _notify(const HEvent<T> & _e) \
    { \
        for ( auto handler : m_eventHandlers_ ## name ) { \
            handler->notify(_e); \
        } \
    } \
    std::vector<std::shared_ptr<EventHandler<T>>> m_eventHandlers_ ## name; \
public: