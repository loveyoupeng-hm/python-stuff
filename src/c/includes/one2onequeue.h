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
        int capacity;
        int slot_size;
        int mask;
        long long padding1[128];
        volatile atomic_long head;
        long cached_tail;
        long long padding2[128];
        volatile atomic_long tail;
        long cached_head;
        long long padding3[128];
        void *data;
    } One2OneQueue;

    One2OneQueue *one2onequeue_new(int capacity, int slot_size);

    bool one2onequeue_offer(One2OneQueue *queue, void *data);
    void *one2onequeue_poll(One2OneQueue *queue);
    int one2onequeue_size(One2OneQueue *queue);

#ifdef __cplusplus
}
#endif
