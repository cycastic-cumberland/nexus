//
// Actually, scratch that! ChatGPT wrote most of this
//

#ifndef NEXUS_THREAD_POOL_H
#define NEXUS_THREAD_POOL_H

#include <functional>
#include <future>
#include <mutex>

#include "managed_thread.h"
#include "../core/types/vector.h"
#include "../core/types/safe_priority_queue.h"

class ThreadPool {
public:
    enum Priority : unsigned char {
        SYSTEM = 0,
        HIGH = 1,
        MEDIUM = 2,
        LOW = 3,
    };
private:
    bool is_terminated;
    std::mutex pool_conditional_mutex{};
    std::condition_variable pool_conditional_lock{};
    Vector<ManagedThread*> all_threads;
    PriorityQueue<std::function<void()>> task_queue{};
    BinaryMutex threads_lock{};

    void allocate_worker_internal(){
        all_threads.emplace(new ManagedThread());
        all_threads.last()->start([this]() -> void {
            while (true){
                std::function<void()> func;
                bool dequeued;
                {
                    std::unique_lock<decltype(pool_conditional_mutex)> lock(pool_conditional_mutex);
                    pool_conditional_lock.wait(lock, [this] { return !task_queue.empty() || is_terminated; });
                    if (is_terminated) return;
                    dequeued = task_queue.try_pop(func);
                }
                if (dequeued) func();
            }
        });
    }
    void init() {
        for (int i = 0; i < all_threads.capacity(); ++i) {
            allocate_worker_internal();
        }
    }
protected:
    _FORCE_INLINE_ void notify_thread() { pool_conditional_lock.notify_one(); }

    template<class T>
    void submit_task(Priority p_priority, const T& p_promise){
        std::unique_lock<decltype(pool_conditional_mutex)> lock(pool_conditional_mutex);
        T promise = p_promise;
        task_queue.push([promise]() {
            (*promise)();
        }, p_priority);
    }
public:
    _FORCE_INLINE_ void allocate_worker(){
        GUARD(threads_lock);
        allocate_worker_internal();
    }
    _FORCE_INLINE_ void batch_allocate_workers(const uint8_t& p_worker_count){
        GUARD(threads_lock);
        for (auto i = 0; i < p_worker_count; i++){
            allocate_worker_internal();
        }
    }

    _FORCE_INLINE_ void terminate_all_workers() {
        GUARD(threads_lock);
        is_terminated = true;
        pool_conditional_lock.notify_all();

        for (auto & m_thread : all_threads) {
            m_thread->join();
            delete m_thread;
        }
        all_threads.clear();
        is_terminated = false;
    }

    // Submit a function to be executed asynchronously by the pool
    template<typename F, typename...Args>
    auto queue_task(Priority p_priority, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        submit_task(p_priority, task_ptr);
        notify_thread();

        // Return future from promise
        return task_ptr->get_future();
    }
    explicit ThreadPool(const uint8_t& n_threads = 3)
            : all_threads(Vector<ManagedThread*>(n_threads)), is_terminated(false) {
        init();
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ~ThreadPool() { terminate_all_workers(); }

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;
};

#endif //NEXUS_THREAD_POOL_H
