//
// Created by cycastic on 8/1/2023.
//

#include <gtest/gtest.h>
#include "../runtime/thread_pool.h"

class ThreadPoolTestFixture : public ::testing::Test {
private:
    ThreadPool *thread_pool{};
    int num = 0;
public:
    void SetUp() override {
        thread_pool = new ThreadPool(3);
    }

    void TearDown() override {
        delete thread_pool;
    }

    std::future<void> queue_task(int p_num){
        return thread_pool->queue_task(ThreadPool::MEDIUM, [this, p_num]() -> void {
            num = p_num;
        });
    }
    bool is_zero() const {
        return num == 0;
    }
    bool is_one() const {
        return num == 1;
    }
};

TEST_F(ThreadPoolTestFixture, TestThreadPool){
    EXPECT_TRUE(is_zero());
    queue_task(1).wait();
    EXPECT_TRUE(is_one());
    queue_task(0).wait();
    EXPECT_TRUE(is_zero());
}
