#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "one2onequeue.h"

extern "C"
{
#include <stdatomic.h>
    typedef struct Data_t
    {
        char *name;
        int age;
    } Data;

    int global_value = 0;

    void match_128(void *, void *data)
    {
        EXPECT_EQ(global_value, *(int *)data);
        global_value++;
    }

    int process_one(void *)
    {
        return 1;
    }

    int process_two(void *, void *)
    {
        return 2;
    }

#define process(...)                                                    \
    _Generic(                                                           \
        (char[2][sizeof((void *[]){__VA_ARGS__}) / sizeof(void *)]){0}, \
        char (*)[1]: process_one,                                       \
        char (*)[2]: process_two)(__VA_ARGS__)
}

namespace test_message_queue
{
    TEST(Utils, atimic)
    {
        ASSERT_EQ(1, process(NULL));
        ASSERT_EQ(2, process(NULL, NULL));
        volatile atomic_int value = 10;
        ASSERT_EQ(10, atomic_load_explicit(&value, memory_order_acquire));
        atomic_store_explicit(&value, 200, memory_order_release);
        ASSERT_EQ(200, atomic_load_explicit(&value, memory_order_acquire));
        int x[10];
        for (int i = 0; i < 10; ++i)
            x[i] = i + 1;
        void **y = (void **)malloc(sizeof(void *) * 10);
        for (int i = 0; i < 10; ++i)
            y[i] = &x[i];
        for (int i = 0; i < 10; ++i)
            ASSERT_EQ(i + 1, *(int *)(y[i]));
    }

    TEST(Libraries, One2OneQueue)
    {
        int n1 = -1;
        unsigned int size = 128;
        One2OneQueue *queue = one2onequeue_new(size, sizeof(int));
        EXPECT_EQ(size, queue->capacity);
        for (int x = 0; x < 10; x++)
        {
            for (int i = 0; i < size; ++i)
            {
                void *value = malloc(sizeof(int));
                *(int *)value = i + x;
                EXPECT_TRUE(one2onequeue_offer(queue, value));
            }
            EXPECT_FALSE(one2onequeue_offer(queue, (void *)&n1));
            for (int i = 0; i < size; ++i)
            {
                EXPECT_EQ(i + x, *(int *)one2onequeue_poll(queue));
            }
            EXPECT_EQ(NULL, one2onequeue_poll(queue));
        }
        for (int x = 0; x < 5; x++)
        {
            global_value = 0;
            for (int i = 0; i < size / 3; ++i)
            {
                void *value = malloc(sizeof(int));
                *(int *)value = i;
                EXPECT_TRUE(one2onequeue_offer(queue, value));
            }
            EXPECT_EQ(size / 3, one2onequeue_drain(queue, NULL, match_128));
        }
        global_value = 0;
        for (int i = 0; i < size; ++i)
        {
            void *value = malloc(sizeof(int));
            *(int *)value = i;
            EXPECT_TRUE(one2onequeue_offer(queue, value));
        }
        EXPECT_FALSE(one2onequeue_offer(queue, (void *)&n1));
        EXPECT_EQ(size / 2, one2onequeue_drain_to(queue, size / 2, NULL, match_128));
        EXPECT_EQ(size / 2, one2onequeue_size(queue));
        EXPECT_EQ(size / 2, one2onequeue_drain_to(queue, size / 2, NULL, match_128));
        EXPECT_EQ(0, one2onequeue_drain_to(queue, size, NULL, match_128));

        free(queue);
    }

    TEST(Library, ThreadCase)
    {
        One2OneQueue *queue = one2onequeue_new(16, sizeof(int));
        int size = 10000;
        for (int x = 0; x < 5; x++)
        {
            int count = 0;
            auto producer = std::thread([&queue, x, size]()
                                        {
            for (int i = 0;i < size;i++){
               while(!one2onequeue_offer(queue, new int{i+ x}));
            } });
            auto consumer = std::thread([&queue, x, size, &count]()
                                        {
            for (int i = 0;i < size;i++){
                int* value = NULL;
                while(NULL == (value = (int*)one2onequeue_poll(queue)));
                ASSERT_EQ(i+x, *value);
                count++;
            } });
            producer.join();
            consumer.join();
            ASSERT_EQ(size, count);
        }
        ASSERT_EQ(2, ATOMIC_LLONG_LOCK_FREE);
    }
}
