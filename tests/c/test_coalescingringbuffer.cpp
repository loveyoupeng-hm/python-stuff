#include <gtest/gtest.h>
#include <thread>
#include "coalescingringbuffer.h"
#include <atomic>

extern "C"
{
#include <stdatomic.h>
    typedef struct context_t
    {
        const unsigned int size;
        const int *values;
        int index;
    } context;

    void test_values(void *con, void *values)
    {
        context *const ctx = (context *)con;
        int *ptr = (int *)values;
        EXPECT_EQ(ctx->values[ctx->index++], *ptr);
    }

    int *create(int size, int start)
    {
        int *result = (int *)malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
            result[i] = start + i;
        return result;
    }

    void larger(void *context, void *value)
    {
    }
}

namespace test_coalescingringbuffer
{
    TEST(Libraries, CoalescingRingBuffer)
    {
        unsigned int size = 1024;
        CoalescingRingBuffer *buffer = coalescingringbuffer_new(size, key_int_equals);
        EXPECT_EQ(size, coalescingringbuffer_capacity(buffer));
        EXPECT_EQ(0, coalescingringbuffer_size(buffer));
        for (int i = 0; i < size / 2; i++)
        {
            void *key = malloc(sizeof(int));
            void *value = malloc(sizeof(int));
            *(int *)key = i;
            *(int *)value = i * 100;
            EXPECT_EQ(true, coalescingringbuffer_offer_key(buffer, key, value));
        }
        for (int i = 0; i < size / 2; i++)
        {
            void *key = malloc(sizeof(int));
            void *value = malloc(sizeof(int));
            *(int *)key = i + 1;
            *(int *)value = i + 1;
            EXPECT_EQ(true, coalescingringbuffer_offer_key(buffer, key, value));
        }
        EXPECT_EQ(size / 2 + 1, coalescingringbuffer_size(buffer));

        int zero = 0;
        context ctx0 = {.size = 1, .values = &zero, .index = 0};
        EXPECT_EQ(1, coalescingringbuffer_poll(buffer, 1, test_values, &ctx0));
        context ctx1 = {.size = size, .values = create(size / 2, 1), .index = 0};
        EXPECT_EQ(size / 2, coalescingringbuffer_poll(buffer, size / 2, test_values, &ctx1));
        EXPECT_EQ(0, coalescingringbuffer_size(buffer));
        free(buffer);
    }

    TEST(Thread, CoalescingRingBuffer)
    {
        const unsigned int size = 128;
        const long loops = 1000000;
        CoalescingRingBuffer *buffer = coalescingringbuffer_new(size, key_int_equals);
        std::atomic_int32_t produced{0};
        std::atomic_int32_t consumed{0};
        auto producer = std::thread([buffer, &produced, loops]()
                                    {
                                        int keys[8] {1,2,3,4,5,6,7,8};
                                        while(produced.load(std::memory_order_consume) < loops)
                                        {
                                            int* key =(int*) malloc(sizeof(int)); 
                                            *key = keys[produced.fetch_add(1, std::memory_order_release)&8];
                                            int * value = (int*) malloc(sizeof(int));
                                            *value = produced.load(std::memory_order_consume); 
                                            while(!coalescingringbuffer_offer_key(buffer, key, value)); 
                                        } });
        auto consumer = std::thread([buffer, &produced, &consumed, loops]()
                                    {
                                        int * context = (int*)malloc(sizeof(int)); 
                                        *context = 0;
                                        while(produced.load(std::memory_order_acquire) < loops){
                                            unsigned int count =  coalescingringbuffer_poll(buffer, 10, larger, context);
                                            consumed.fetch_add(count, std::memory_order_release);
                                    } });
        consumer.join();
        producer.join();
        free(buffer);
        EXPECT_TRUE(consumed.load(std::memory_order_acquire) <= loops);
        EXPECT_TRUE(consumed.load(std::memory_order_acquire) >= loops / 100);
    }
}
