//
// Created by cycastic on 8/1/2023.
//

#ifndef NEXUS_COMMAND_QUEUE_H
#define NEXUS_COMMAND_QUEUE_H

#include "thread_pool.h"
#include "../core/types/queue.h"

class CommandQueue {
private:
    bool is_terminated;
    std::mutex conditional_mutex{};
    std::condition_variable conditional_lock{};
    Queue<std::function<void()>> task_queue{};
    ManagedThread server;

    template<class T>
    _FORCE_INLINE_ auto dispatch_internal(const std::function<T>& p_func){
        auto task_ptr = std::make_shared<std::packaged_task<T>>(p_func);
        {
            std::unique_lock<decltype(conditional_mutex)> lock(conditional_mutex);
            task_queue.enqueue([task_ptr]() {
                (*task_ptr)();
            });
        }
        conditional_lock.notify_one();
        return task_ptr->get_future();
    }
public:
    CommandQueue() : server(), is_terminated(false) {
        server.start([this]() {
            while (true){
                std::function<void()> func;
                {
                    std::unique_lock<decltype(conditional_mutex)> lock(conditional_mutex);
                    conditional_lock.wait(lock, [this] { return !task_queue.empty() || is_terminated; });
                    if (is_terminated) return;
                    if (task_queue.empty()) continue;
                    func = task_queue.dequeue();
                }
                func();
            }
        });
    }
    ~CommandQueue() {
        is_terminated = true;
        conditional_lock.notify_all();
        server.join();
    }
    _FORCE_INLINE_ ManagedThread::ID get_server_id() { return server.get_id(); }

    template<typename F, typename...Args>
    auto dispatch(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        dispatch_internal(func);
    }
    template<typename T, typename F, typename...Args>
    auto dispatch_method(T* p_instance, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype((p_instance->*f)(args...))()> func = std::bind(std::forward<F>(f), p_instance, std::forward<Args>(args)...);
        dispatch_internal(func);
    }
    template<typename T, typename F, typename...Args>
    auto dispatch_method(const T* p_instance, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype((p_instance->*f)(args...))()> func = std::bind(std::forward<F>(f), p_instance, std::forward<Args>(args)...);
        dispatch_internal(func);
    }
};

#endif //NEXUS_COMMAND_QUEUE_H
