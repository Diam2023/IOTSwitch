#pragma once


#include <functional>
#include <atomic>
#include <list>
#include <memory>

template<typename D>
class Listener {
public:
    virtual void onListener(const D &d) = 0;

    Listener() = default;

    void operator()(const D &d) {
        onListener(d);
    }

    virtual ~Listener() = default;
};


/**
 * LiveData class To listener Data Change
 * @tparam D Data Type
 */
template<typename D>
class LiveData {
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

    explicit LiveData(const D &d) : data(d) {};

    /**
     * Remove From Listener list
     */
    void remove(const std::function<void(const D &)> &);

    template<typename F>
    void append(F &&);

    /**
     * Clear All Listener
     */
    void clear();

    /**
     * Return Data
     * @return
     */
    std::atomic<D> &operator*() {
        return data;
    }

    LiveData<D> &operator=(const D &d) {
        data = d;
        notify(d);
        return *this;
    }
};

template<typename D>
inline void LiveData<D>::clear() {
    listeners.clear();
}

template<typename D>
template<typename F>
inline void LiveData<D>::append(F &&f) {
    listeners.push_back(f);
}

template<typename D>
inline void LiveData<D>::notify(const D &d) {
    for (auto &l: listeners) {
        l(d);
    }
}

template<typename D>
inline void LiveData<D>::remove(const std::function<void(const D &)> &l) {
    listeners.remove(l);
}




