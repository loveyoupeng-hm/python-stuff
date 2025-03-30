#include <stdlib.h>
#include <threads.h>
#include <stdatomic.h>
#include "one2onequeue.h"
#include <string.h>

One2OneQueue *one2onequeue_new(int capacity, int size)
{
    One2OneQueue *result = (One2OneQueue *)malloc(sizeof(One2OneQueue));
    result->capacity = capacity;
    result->mask = capacity - 1;
    result->size = size;
    result->cached_head = 0;
    result->cached_tail = 0;
    result->data = (void *)malloc(size * capacity);
    atomic_store_explicit(&(result->head), 0, memory_order_release);
    atomic_store_explicit(&(result->tail), 0, memory_order_release);
    return result;
}

bool try_offer(One2OneQueue *queue, void *data)
{
    long limit = queue->cached_tail + queue->capacity;
    if (queue->head >= limit)
    {
        queue->cached_tail = queue->cached_tail;
        limit = queue->cached_tail + queue->capacity;
    }

    if (queue->head >= limit)
    {
        return false;
    }
    memcpy(&queue->data[queue->head & queue->mask], data, queue->size);
    atomic_store_explicit(&(queue->head), queue->head + 1, memory_order_release);
    return true;
}