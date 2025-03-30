#include "one2onequeue.h"
#include <threads.h>
#include <stdatomic.h>

One2OneQueue *one2onequeue_new(int capacity, int size)
{
    One2OneQueue *result = (One2OneQueue *)malloc(sizeof(One2OneQueue));
    result->capacity = capacity;
    result->cached_head = 0;
    result->cached_tail = 0;
    result->data = (void *)malloc(size * capacity);
    atomic_store_explicit(&(result->head), 0, memory_order_release);
    atomic_store_explicit(&(result->tail), 0, memory_order_release);
    return result;
}