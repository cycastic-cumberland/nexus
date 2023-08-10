//
// Created by cycastic on 8/1/2023.
//

#include <gtest/gtest.h>
#include "../runtime/thread_pool.h"
#include "../runtime/utils.h"

class ThreadPoolTestFixture : public ::testing::Test {
private:
    ThreadPool *thread_pool{};
    int num = 0;
    void set_num(const int& p_num){
        num = p_num;
    }
public:
    void SetUp() override {
        thread_pool = new ThreadPool(3);
    }

    void TearDown() override {
        delete thread_pool;
    }

    std::future<void> queue_task(int p_num){
        return thread_pool->queue_task_method(ThreadPool::MEDIUM, this, &ThreadPoolTestFixture::set_num, p_num);
    }
    bool is_zero() const {
        return num == 0;
    }
    bool is_one() const {
        return num == 1;
    }
    bool allocation_test(){
        auto old_size = thread_pool->get_thread_count();
        thread_pool->allocate_worker();
        return thread_pool->get_thread_count() == old_size + 1;
    }
    bool deallocation_test(){
        auto old_size = thread_pool->get_thread_count();
        if (old_size == 0) return true;
        thread_pool->terminate_worker();
        // Wait for it to take effect
        ManagedThread::sleep(1000);
        return thread_pool->get_thread_count() == old_size - 1;
    }
    static bool integrity_check(const char* p_left, const char* p_right, const size_t& size) {
        for (size_t i = 0; i < size; i++){
            if (p_left[i] != p_right[i]) return false;
        }
        return true;
    }
    ThreadPool* get_pool() { return thread_pool; }
};

TEST_F(ThreadPoolTestFixture, TestThreadPool){
    static constexpr auto size = 1024 * 1024 * 64;
    EXPECT_TRUE(allocation_test());
    EXPECT_TRUE(deallocation_test());
    EXPECT_TRUE(is_zero());
    queue_task(1).wait();
    EXPECT_TRUE(is_one());
    queue_task(0).wait();
    EXPECT_TRUE(is_zero());

    char* a = new char[size];
    char* b = new char[size];

    for (size_t i = 0; i < size; i++)
        a[i] = 42;

    NexusUtils::batch_copy(get_pool(), ThreadPool::MEDIUM, get_pool()->get_thread_count() * 2, b, a, size);
    EXPECT_TRUE(integrity_check(a, b, size));

    delete[] a;
    delete[] b;
}
