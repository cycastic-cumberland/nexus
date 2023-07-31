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
    auto queue_task(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        {
            std::unique_lock<decltype(conditional_mutex)> lock(conditional_mutex);
            task_queue.enqueue([task_ptr]() {
                (*task_ptr)();
            });
        }
        conditional_lock.notify_one();
        return task_ptr->get_future();
    }
};

#endif //NEXUS_COMMAND_QUEUE_H
