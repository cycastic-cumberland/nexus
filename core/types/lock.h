//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_LOCK_H
#define NEXUS_LOCK_H

#include <mutex>
#include <shared_mutex>
#include "reference.h"

class Lock : public Object {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool try_lock() = 0;
};

class BinaryMutex : public Lock {
private:
    std::mutex _mutex{};
public:
    void lock() override { _mutex.lock(); }
    void unlock() override { _mutex.unlock(); }
    bool try_lock() override { return _mutex.try_lock(); }
};

class SharedMutex : public Lock {
private:
    std::shared_mutex _mutex{};
public:
    void lock() override { _mutex.lock(); }
    void unlock() override { _mutex.unlock(); }
    bool try_lock() override { return _mutex.try_lock(); }
};

class LockGuard {
    Lock* lock;
public:
    explicit LockGuard(Lock* p_lock) { lock = p_lock; lock->lock(); }
    ~LockGuard() {
        lock->unlock();
    }
};

#endif //NEXUS_LOCK_H
