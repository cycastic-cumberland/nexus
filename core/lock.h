//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_LOCK_H
#define NEXUS_LOCK_H

#include <mutex>
#include <shared_mutex>
#ifdef DEBUG_ENABLED
#include <thread>
#endif

class Lock {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool try_lock() = 0;
};

class InertLock : Lock {
public:
    void lock() override {}
    void unlock() override {}
    bool try_lock() override { return true; }
};

class BinaryMutex : public Lock {
private:
    std::mutex _mutex{};
#ifdef DEBUG_ENABLED
    std::thread::id acquired_by{};
#endif
public:
    void lock() override {
        _mutex.lock();
#ifdef DEBUG_ENABLED
        acquired_by = std::this_thread::get_id();
#endif
    }
    void unlock() override {
#ifdef DEBUG_ENABLED
        acquired_by = std::thread::id();
#endif
        _mutex.unlock();
    }
    bool try_lock() override {
#ifdef DEBUG_ENABLED
        auto success = _mutex.try_lock();
        if (success) acquired_by = std::thread::id();
        return success;
#else
        return _mutex.try_lock();
#endif
    }

#ifdef DEBUG_ENABLED
      std::thread::id currently_acquired_by() const {
        return acquired_by;
    }
#endif
};

class SharedMutex : public Lock {
private:
    std::shared_mutex _mutex{};
#ifdef DEBUG_ENABLED
    std::thread::id acquired_by{};
#endif
public:
    void lock() override {
        _mutex.lock();
#ifdef DEBUG_ENABLED
        acquired_by = std::this_thread::get_id();
#endif
    }
    void unlock() override {
#ifdef DEBUG_ENABLED
        acquired_by = std::thread::id();
#endif
        _mutex.unlock();
    }
    bool try_lock() override {
#ifdef DEBUG_ENABLED
        auto success = _mutex.try_lock();
        if (success) acquired_by = std::thread::id();
        return success;
#else
        return _mutex.try_lock();
#endif
    }

#ifdef DEBUG_ENABLED
      std::thread::id currently_acquired_by() const {
        return acquired_by;
    }
#endif
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
#ifdef DEBUG_ENABLED
    std::thread::id acquired_by{};
#endif
public:
    virtual void read_lock() const {
        mutex.lock_shared();
    }
    virtual void read_unlock() const {
        mutex.unlock_shared();
    }
    virtual bool read_try_lock() const {
        return mutex.try_lock_shared();
    }
    virtual void write_lock() {
        mutex.lock();
#ifdef DEBUG_ENABLED
        acquired_by = std::this_thread::get_id();
#endif
    }
    virtual void write_unlock() {
#ifdef DEBUG_ENABLED
        acquired_by = std::thread::id();
#endif
        mutex.unlock();
    }
    virtual bool write_try_lock() {
#ifdef DEBUG_ENABLED
        auto success = mutex.try_lock();
        if (success) acquired_by = std::thread::id();
        return success;
#else
        return mutex.try_lock();
#endif
    }
    RWLock() : mutex() {

    }
};

class InertRWLock : public RWLock {
public:
    void read_lock() const override {}
    void read_unlock() const override {}
    bool read_try_lock() const override { return true; }
    void write_lock() override {}
    void write_unlock() override {}
    bool write_try_lock() override { return true; }
    InertRWLock() : RWLock() {}
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

#include <functional>

template <class T>
class ReaderWriter {
private:
    T object;
    mutable RWLock lock;
public:
    template <class ...Args>
    ReaderWriter(Args&& ...args) : object(args...), lock() {}
    ReaderWriter(const T& p_other) : object(p_other), lock() {}

    void read(std::function<void(const T&)> cb) const {
        lock.read_lock();
        cb(object);
        lock.read_unlock();
    }
    template <typename TR>
    TR rread(std::function<TR(const T&)> cb) const {
        ReadLockGuard guard(lock);
        return cb(object);
    }
    void write(std::function<void(T&)> cb) {
        lock.write_lock();
        cb(object);
        lock.write_unlock();
    }
    template <typename TR>
    TR rwrite(std::function<TR(T&)> cb) {
        WriteLockGuard guard(lock);
        return cb(object);
    }
};

#endif //NEXUS_LOCK_H
