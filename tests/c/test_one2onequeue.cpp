#include <gtest/gtest.h>
#include "one2onequeue.h"
#include <atomic>

namespace test_message_queue
{

    TEST(Libraries, One2OneQueue)
    {
        int n1 = -1;
        int size = 2;
        One2OneQueue *queue = one2onequeue_new(size, sizeof(int));
        EXPECT_EQ(size, queue->capacity);
        for (int i = 0; i < size; ++i)
        {
            EXPECT_TRUE(try_offer(queue, (void *)&i));
        }
        EXPECT_FALSE(try_offer(queue, (void *)&n1));
        free(queue);
        EXPECT_EQ(1, 3 - 2);
    }

}
