#include <stdlib.h>
#include <threads.h>
#include <stdatomic.h>
#include "one2onequeue.h"
#include <string.h>

One2OneQueue *one2onequeue_new(int capacity, int slot_size)
{
    One2OneQueue *result = (One2OneQueue *)malloc(sizeof(One2OneQueue));
    result->capacity = capacity;
    result->slot_size = slot_size;
    result->mask = capacity - 1;
    result->cached_head = 0;
    result->cached_tail = 0;
    result->data = (void *)malloc(capacity * slot_size);
    atomic_store_explicit(&(result->head), 0, memory_order_release);
    atomic_store_explicit(&(result->tail), 0, memory_order_release);
    return result;
}

bool one2onequeue_offer(One2OneQueue *queue, void *data)
{
    long limit = queue->cached_tail + queue->capacity;
    long next = queue->head;
    if (next >= limit)
    {
        atomic_load_explicit(&queue->tail, memory_order_acquire);
        queue->cached_tail = queue->tail;
        limit = queue->cached_tail + queue->capacity;
    }

    if (next >= limit)
    {
        return false;
    }
    void *loc = (char *)queue->data + queue->slot_size * (next & queue->mask);
    memcpy(loc, data, queue->slot_size);
    atomic_store_explicit(&(queue->head), next + 1, memory_order_release);
    return true;
}

void *one2onequeue_poll(One2OneQueue *queue)
{
    long next = queue->tail;
    if (next >= queue->cached_head)
    {
        atomic_load_explicit(&queue->head, memory_order_acquire);
        queue->cached_head = queue->head;
    }
    if (next >= queue->cached_head)
    {
        return NULL;
    }
    void *result = malloc(queue->slot_size);
    void *loc = (char *)queue->data + queue->slot_size * (next & queue->mask);
    memcpy(result, loc, queue->slot_size);
    atomic_store_explicit(&queue->tail, next + 1, memory_order_release);
    return result;
}

int one2onequeue_size(One2OneQueue *queue)
{
    return queue->head - queue->tail;
}