#include <stdlib.h>
#include <threads.h>
#include <stdatomic.h>
#include "one2onequeue.h"
#include <string.h>

One2OneQueue *one2onequeue_new(int capacity)
{
    One2OneQueue *result = (One2OneQueue *)malloc(sizeof(One2OneQueue));
    result->capacity = capacity;
    result->mask = capacity - 1;
    result->cached_head = 0;
    result->cached_tail = 0;
    result->data = (void **)malloc(capacity * sizeof(void *));
    atomic_store_explicit(&(result->head), 0, memory_order_release);
    atomic_store_explicit(&(result->tail), 0, memory_order_release);
    return result;
}

bool one2onequeue_offer(One2OneQueue *queue, void *data)
{
    long limit = queue->cached_tail + queue->capacity;
    if (queue->head >= limit)
    {
        queue->cached_tail = atomic_load_explicit(&queue->tail, memory_order_acquire);
        limit = queue->cached_tail + queue->capacity;
    }

    if (queue->head >= limit)
    {
        return false;
    }
    queue->data[queue->head & queue->mask] = data;
    atomic_store_explicit(&(queue->head), queue->head + 1, memory_order_release);
    return true;
}

void *one2onequeue_poll(One2OneQueue *queue)
{
    long tail = queue->tail;
    if (tail >= queue->cached_head)
    {
        queue->cached_head = atomic_load_explicit(&queue->head, memory_order_acquire);
    }
    if (tail >= queue->cached_head)
    {
        return NULL;
    }
    void *result = queue->data[tail & queue->mask];
    queue->data[tail & queue->mask] = NULL;
    atomic_store_explicit(&queue->tail, tail + 1, memory_order_release);
    return result;
}