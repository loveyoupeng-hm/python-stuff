#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct One2OneQueue_t
{
    int capacity;
    long long padding1[128];
    volatile atomic_long head;
    long cached_tail;
    long long padding2[128];
    volatile atomic_long tail;
    long cached_head;
    long long padding3[128];
    void *data;
} One2OneQueue;

One2OneQueue *one2onequeue_new(int capacity, int size);

#ifdef __cplusplus
}
#endif
