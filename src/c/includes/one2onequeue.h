#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <threads.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct One2OneQueue_t
    {
        unsigned int capacity;
        unsigned int slot_size;
        unsigned int mask;
        long long padding1[128];
        volatile atomic_ullong head;
        long long cached_tail;
        long long padding2[128];
        volatile atomic_ullong tail;
        unsigned long long cached_head;
        long long padding3[128];
        void *data;
    } One2OneQueue;

    One2OneQueue *one2onequeue_new(unsigned int capacity, unsigned int slot_size);

    bool one2onequeue_offer(One2OneQueue *queue, void *data);
    void *one2onequeue_poll(One2OneQueue *queue);
    unsigned int one2onequeue_size(One2OneQueue *queue);
    unsigned int one2onequeue_drain_to(One2OneQueue *queue, unsigned int size, void *context, void (*func)(void *, void *));
    inline unsigned int one2onequeue_drain(One2OneQueue *queue, void *context, void (*func)(void *, void *)) { return one2onequeue_drain_to(queue, UINT_MAX, context, func); }

#ifdef __cplusplus
}
#endif
