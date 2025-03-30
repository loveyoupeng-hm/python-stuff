#include <gtest/gtest.h>
#include "one2onequeue.h"
#include <atomic>

namespace test_message_queue
{

    TEST(Libraries, One2OneQueue)
    {
        One2OneQueue *queue = one2onequeue_new(128, sizeof(int));
        EXPECT_EQ(128, queue->capacity);
        free(queue);
        EXPECT_EQ(1, 3 - 2);
    }

}
