#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

template <typename D>
class Listener
{
public:
    virtual void onListener(const D &d) = 0;

    Listener() = default;

    void operator()(const D &d)
    {
        onListener(d);
    }

    virtual ~Listener() = default;
};

/**
 * LiveData class To listener Data Change
 * @tparam D Data Type
 */
template <typename D>
class LiveData
{
private:
    void notify(const D &);

protected:
    /**
     * Data For LiveData
     */
    std::atomic<D> data;

    /**
     * Listener list
     */
    std::list<std::function<void(const D &)>> listeners;

public:
    LiveData() = default;

    explicit LiveData(const D &d) : data(d){};

    /**
     * Remove From Listener list
     */
    void remove(const std::function<void(const D &)> &);

    template <typename F>
    void append(F &&);

    /**
     * Clear All Listener
     */
    void clear();

    /**
     * Return Data
     * @return
     */
    std::atomic<D> &operator*()
    {
        return data;
    }

    virtual LiveData<D> &operator=(const D &d)
    {
        data = d;
        notify(d);
        return *this;
    }
};

template <typename D>
inline void LiveData<D>::clear()
{
    listeners.clear();
}

template <typename D>
template <typename F>
inline void LiveData<D>::append(F &&f)
{
    listeners.push_back(f);
}

template <typename D>
inline void LiveData<D>::notify(const D &d)
{
    for (auto &l : listeners)
    {
        l(d);
    }
}

template <typename D>
inline void LiveData<D>::remove(const std::function<void(const D &)> &l)
{
    listeners.remove(l);
}

using namespace std::chrono_literals;

/**
 * AsyncData class To listener Data Change
 * @tparam D Data Type
 */
template <typename D>
class AsyncData
{
protected:
    /**
     * Data For LiveData
     */
    std::atomic<D> data;

    std::chrono::nanoseconds loopTime;

    /**
     * Listener list
     */
    std::list<std::function<void(const D &)>> listeners;

    /**
     * Changed Data Collection
     */
    std::queue<D> feedDataQueue;

    std::mutex feedDataMutex;

    //    std::condition_variable feedDataCond;

    /**
     * AsyncData Thread
     */
    std::thread thread;

public:
    void run()
    {
        while (true)
        {
            if (!feedDataQueue.empty())
            {
                if (feedDataMutex.try_lock())
                {
                    data = feedDataQueue.front();
                    feedDataQueue.pop();
                    feedDataMutex.unlock();

                    // call it
                    for (const auto &l : listeners)
                    {
                        l(data);
                    }
                }
            }
            //            D d;
            //            {
            //                std::unique_lock<std::mutex> ul(feedDataMutex);
            //                feedDataCond.wait(ul,
            //                                  [this]() { return
            //                                  !feedDataQueue.empty(); });
            //                d = feedDataQueue.front();
            //                feedDataQueue.pop();
            //            }    // Release Lock
            //            data = d;


            std::this_thread::sleep_for(loopTime);
        }
    }

    // Delete Default And Copy Construction
    AsyncData()                  = delete;
    AsyncData(AsyncData &&)      = delete;
    AsyncData(const AsyncData &) = delete;

    explicit AsyncData(const D &d, const std::chrono::nanoseconds &l = 2000ns)
        : data(d), loopTime(l), thread(&AsyncData::run, this){};

    /**
     * Remove From Listener list
     */
    void remove(const std::function<void(const D &)> &f)
    {
        listeners.erase(std::remove_if(listeners.begin(),
                                       listeners.end(),
                                       [f](const auto &f_) -> bool {
                                           return f == f_;
                                       }),
                        listeners.end());
    }

    template <typename F>
    void append(F &&f)
    {
        listeners.push_back(f);
    }

    /**
     * Return Now Data
     * @return
     */
    std::atomic<D> &operator*()
    {
        return data;
    }

    /**
     * Sync Maybe!
     * @param d
     * @return
     */
    virtual AsyncData<D> &operator=(const D &d)
    {
        feedDataMutex.lock();
//        std::lock_guard<std::mutex> lg(feedDataMutex);
        feedDataQueue.push(d);
        feedDataMutex.unlock();
//        this->feedDataCond.notify_one();
        return *this;
    }
};