#include <benchmark/benchmark.h>
#include <iostream>
#include <thread>
extern "C"
{
#include "one2onequeue.h"

    void doNonthing(void *, void *) {}
}
class One2OneQueueFixture : public ::benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State &state) override
    {
        if (state.thread_index() == 0)
        {
            queue = one2onequeue_new(state.range(0), sizeof(int));
            atomic_thread_fence(memory_order_release);
            atomic_store_explicit(&flag, 1, memory_order_release);
        }
        else
        {
            while (0 == atomic_load_explicit(&flag, memory_order_acquire) || queue == NULL)
                ;
        }
    }

    void TearDown(const ::benchmark::State &state) override
    {
        if (state.thread_index() == 0)
        {
            free(queue);
        }
        queue = NULL;
    }

    ~One2OneQueueFixture() override { assert(queue == NULL); }

    volatile atomic_int flag = 0;
    One2OneQueue *queue = NULL;

    int value = 100;
};

BENCHMARK_DEFINE_F(One2OneQueueFixture, One2OneQueue)(benchmark::State &state)
{

    long long offer = 0;
    long long poll = 0;
    long long total_offer = 0;
    long long total_poll = 0;
    for (auto _ : state)
    {
        if (state.thread_index() == 0)
        {
            value++;
            if (one2onequeue_offer(queue, &value))
                offer++;
            total_offer++;
        }
        else
        {
            void *result = one2onequeue_poll(queue);
            benchmark::DoNotOptimize(result);
            if (NULL != result)
                poll++;
            total_poll++;
        }
    }
    if (state.thread_index() == 0)
    {

        state.counters["offer"] = offer;
        state.counters["total_offer"] = total_offer;
        state.counters["offer_ratio"] = (double)offer / (double)total_offer;
    }
    else
    {
        state.counters["poll"] = poll;
        state.counters["total_poll"] = total_poll;
        state.counters["poll_ratio"] = (double)poll / (double)total_poll;
    }
}

BENCHMARK_REGISTER_F(One2OneQueueFixture, One2OneQueue)->RangeMultiplier(2)->Range(2, 1 << 10)->Threads(2);

BENCHMARK_DEFINE_F(One2OneQueueFixture, One2OneQueueDo)(benchmark::State &state)
{

    void *result = NULL;
    for (auto _ : state)
    {
        if (state.thread_index() == 0)
        {
            while (!one2onequeue_offer(queue, &value))
                ;
        }
        else
        {
            while (NULL == (result = one2onequeue_poll(queue)))
                ;
            benchmark::DoNotOptimize(result);
        }
    }
}

BENCHMARK_REGISTER_F(One2OneQueueFixture, One2OneQueueDo)->RangeMultiplier(2)->Range(2, 1 << 10)->Threads(2);

class One2OneQueueOfferFixture : public ::benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State &state) override
    {
        queue = one2onequeue_new(state.range(0), sizeof(int));
        atomic_store_explicit(&flag, 1, memory_order_release);
        consumer = std::thread{&One2OneQueueOfferFixture::run, this};
    }

    void run()
    {
        while (1 == atomic_load_explicit(&flag, memory_order_acquire))
            one2onequeue_drain(queue, NULL, &doNonthing);
    }

    void TearDown(const ::benchmark::State &state) override
    {
        atomic_store_explicit(&flag, 0, memory_order_release);
        consumer.join();
        free(queue);
        queue = NULL;
    }

    ~One2OneQueueOfferFixture() override { assert(queue == NULL); }

    volatile atomic_int flag = 0;
    One2OneQueue *queue = NULL;
    std::thread consumer;
    int value = 100;
};

BENCHMARK_DEFINE_F(One2OneQueueOfferFixture, One2OneQueueDrain)(benchmark::State &state)
{
    long long success = 0;
    long long fail = 0;

    for (auto _ : state)
    {
        while (!one2onequeue_offer(queue, &value))
            fail++;
        success++;
    }
    state.counters["success"] = success;
    state.counters["fail"] = fail;
    state.counters["offer_ratio"] = (double)success / (double)(success + fail);
}

BENCHMARK_REGISTER_F(One2OneQueueOfferFixture, One2OneQueueDrain)->RangeMultiplier(2)->Range(2, 1 << 10);
BENCHMARK_MAIN();