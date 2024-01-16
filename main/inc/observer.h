#pragma once


#include <functional>
#include <atomic>
#include <list>
#include <memory>

template<typename D>
class Listener {
protected:
    std::function<void(const D &)> callback;
public:
    virtual void onListener(const D &d) {

    };

    Listener() = default;

    template<typename F>
    Listener(F &&f) : callback(std::forward<F>(f)) {};

    virtual ~Listener() = default;
};


/**
 * LiveData class To listener Data Change
 * @tparam D Data Type
 */
template<typename D>
class LiveData {
protected:

    /**
     * Data For LiveData
     */
    std::atomic<D> data;

    /**
     * Listener list
     */
    std::list<Listener<D>> listeners;
public:
    LiveData() = default;

    explicit LiveData(const D &d) : data(d) {};

    /**
     * Remove From Listener list
     */
    void remove(const Listener<D> &);

    void append(const Listener<D> &);

    void notify(const D &);

    /**
     * Return Data
     * @return
     */
    std::atomic<D> operator*() {
        return data;
    }

    LiveData<D> &operator=(const D &d) {
        data = d;
        notify(d);
        return *this;
    }
};

template<typename D>
void LiveData<D>::notify(const D &d) {
    for (auto &l: listeners) {
        l.onListener(d);
    }
}

template<typename D>
void LiveData<D>::append(const Listener<D> &l) {
    listeners.push_back(l);
}

template<typename D>
inline void LiveData<D>::remove(const Listener<D> &l) {
    listeners.remove(l);
}




