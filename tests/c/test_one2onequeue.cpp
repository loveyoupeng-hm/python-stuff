#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "one2onequeue.h"

extern "C"
{
    typedef struct Data_t
    {
        char *name;
        int age;
    } Data;
}

namespace test_message_queue
{
    TEST(Libraries, One2OneQueue)
    {
        int n1 = -1;
        int size = 128;
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
        free(queue);
        EXPECT_EQ(1, 3 - 2);
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
    }
}
