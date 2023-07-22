//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_LOCK_H
#define NEXUS_LOCK_H

#include <mutex>
#include <shared_mutex>

class Lock {
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
private:
    Lock* inner_lock;
public:
    explicit LockGuard(Lock* lock) {
        inner_lock = lock;
        inner_lock->lock();
    }
    explicit LockGuard(Lock& lock) : LockGuard(&lock) {}
    ~LockGuard() {
        inner_lock->unlock();
    }
};

class RWLock {
    mutable std::shared_timed_mutex mutex;

public:
    void read_lock() const {
        mutex.lock_shared();
    }
    void read_unlock() const {
        mutex.unlock_shared();
    }
    bool read_try_lock() const {
        return mutex.try_lock_shared();
    }
    void write_lock() {
        mutex.lock();
    }
    void write_unlock() {
        mutex.unlock();
    }
    bool write_try_lock() {
        return mutex.try_lock();
    }
};

class ReadLockGuard {
private:
    RWLock* inner_lock;
public:
    explicit ReadLockGuard(RWLock* lock) {
        inner_lock = lock;
        inner_lock->read_lock();
    }
    explicit ReadLockGuard(RWLock& lock) : ReadLockGuard(&lock) {}
    ~ReadLockGuard() {
        inner_lock->read_unlock();
    }
};

class WriteLockGuard {
private:
    RWLock* inner_lock;
public:
    explicit WriteLockGuard(RWLock* lock) {
        inner_lock = lock;
        inner_lock->write_lock();
    }
    explicit WriteLockGuard(RWLock& lock) : WriteLockGuard(&lock) {}
    ~WriteLockGuard() {
        inner_lock->write_unlock();
    }
};

#endif //NEXUS_LOCK_H
