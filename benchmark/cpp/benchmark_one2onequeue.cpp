#include <benchmark/benchmark.h>
#include <iostream>
extern "C"
{
#include "one2onequeue.h"
}
class One2OneQueueFixture : public ::benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State &state) override
    {
        if (state.thread_index() == 0)
        {
            queue = one2onequeue_new(state.range(0));
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

    One2OneQueue *queue;
    int value = 100;
};

BENCHMARK_DEFINE_F(One2OneQueueFixture, One2OneQueue)(benchmark::State &state)
{

    long offer = 0;
    long poll = 0;
    long total_offer = 0;
    long total_poll = 0;
    for (auto _ : state)
    {
        if (state.thread_index() == 0)
        {
            if (one2onequeue_offer(queue, &value))
                offer++;
            total_offer++;
        }
        else
        {
            if (NULL != one2onequeue_poll(queue))
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

BENCHMARK_REGISTER_F(One2OneQueueFixture, One2OneQueue)->RangeMultiplier(2)->Range(2, 1 << 15)->Threads(2);

BENCHMARK_MAIN();