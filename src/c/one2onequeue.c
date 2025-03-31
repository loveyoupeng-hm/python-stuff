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
    memset(result->data, 0, capacity * slot_size);
    atomic_store_explicit(&(result->head), 0, memory_order_release);
    atomic_store_explicit(&(result->tail), 0, memory_order_release);
    return result;
}

bool one2onequeue_offer(One2OneQueue *queue, void *data)
{
    long currentHead = queue->cached_head;
    const int capacity = queue->capacity ;
    long limit = currentHead + capacity;
    const long currentTail = queue->tail;

    if (currentTail >= limit)
    {
        atomic_load_explicit(&queue->head, memory_order_acquire);
        currentHead = queue->head;
        limit = currentHead + capacity;
        if (currentTail >= limit)
            return false;
        queue->cached_head = currentHead;
    }

    void *loc = (char *)queue->data + queue->slot_size * (currentTail & queue->mask);
    memcpy(loc, data, queue->slot_size);
    atomic_store_explicit(&(queue->tail), currentTail + 1, memory_order_release);
    return true;
}

void *one2onequeue_poll(One2OneQueue *queue)
{
    const long currentHead = queue->head;
    long currentTail = queue->cached_tail;
    if (currentHead >= currentTail)
    {
        atomic_load_explicit(&queue->tail, memory_order_acquire);
        currentTail = queue->tail;
        if (currentHead >= currentTail)
            return NULL;
        queue->cached_tail = currentTail;
    }
    void *result = malloc(queue->slot_size);
    void *loc = (char *)queue->data + queue->slot_size * (currentHead & queue->mask);
    memcpy(result, loc, queue->slot_size);
    memset(loc, 0, queue->slot_size);
    atomic_store_explicit(&queue->head, currentHead + 1, memory_order_release);
    return result;
}

int one2onequeue_size(One2OneQueue *queue)
{
    return queue->tail - queue->head;
}