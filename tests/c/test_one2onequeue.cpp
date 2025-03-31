#include <gtest/gtest.h>
#include "one2onequeue.h"
#include <atomic>

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
    TEST(Util, memcpy)
    {
        void *buffer = malloc(sizeof(Data) * 8);
        char name[3] = {'a', 'b', 'c'};
        Data data = {
            .name = name,
            .age = 30};
        void *loc = (char *)buffer + sizeof(Data) * 3;
        memcpy(loc, &data, sizeof(Data));
        ASSERT_EQ(data.age, ((Data *)loc)->age);
        ASSERT_EQ(0, strcmp(data.name, ((Data *)loc)->name));
    }

    TEST(Libraries, One2OneQueue)
    {
        int n1 = -1;
        int size = 128;
        One2OneQueue *queue = one2onequeue_new(size);
        EXPECT_EQ(size, queue->capacity);
        for (int i = 0; i < size; ++i)
        {
            void *value = malloc(sizeof(int));
            *(int *)value = i;
            EXPECT_TRUE(one2onequeue_offer(queue, value));
        }
        EXPECT_FALSE(one2onequeue_offer(queue, (void *)&n1));
        for (int i = 0; i < size; ++i)
        {
            EXPECT_EQ(i, *(int *)one2onequeue_poll(queue));
        }
        EXPECT_EQ(NULL, one2onequeue_poll(queue));
        free(queue);
        EXPECT_EQ(1, 3 - 2);
    }
}
