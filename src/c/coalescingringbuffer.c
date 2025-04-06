#include "coalescingringbuffer.h"

CoalescingRingBuffer *coalescingringbuffer_new(unsigned int capacity, bool (*key_equals)(void *, void *))
{
    CoalescingRingBuffer *this = malloc(sizeof(CoalescingRingBuffer));
    this->capacity = capacity;
    this->mask = capacity - 1;
    this->keys = malloc(sizeof(void *) * capacity);
    atomic_init(&this->values, malloc(sizeof(void *) * capacity));
    for (int i = 0; i < capacity; ++i)
        atomic_init(&this->values[i], NULL);
    atomic_store_explicit(&this->next_write, 1, memory_order_release);
    atomic_store_explicit(&this->first_write, 1, memory_order_release);
    atomic_store_explicit(&this->rejection_count, 0, memory_order_release);
    atomic_store_explicit(&this->last_read, 0, memory_order_release);
    this->key_equals = key_equals;
    return this;
}

unsigned int coalescingringbuffer_size(CoalescingRingBuffer *buffer)
{
    while (true)
    {
        unsigned long long last_read_before = atomic_load_explicit(&buffer->last_read, memory_order_acquire);
        unsigned long long current_next_write = atomic_load_explicit(&buffer->next_write, memory_order_acquire);
        unsigned long long last_read_after = atomic_load_explicit(&buffer->last_read, memory_order_acquire);
        if (last_read_before == last_read_after)
            return (unsigned int)(current_next_write - last_read_before) - 1;
    }
}

unsigned int coalescingringbuffer_capacity(CoalescingRingBuffer *buffer)
{
    return buffer->capacity;
}
unsigned long long coalescingringbuffer_rejection_count(CoalescingRingBuffer *buffer)
{
    return atomic_load_explicit(&buffer->rejection_count, memory_order_acquire);
}

bool coalescingringbuffer_is_empty(CoalescingRingBuffer *buffer)
{
    return atomic_load_explicit(&buffer->first_write, memory_order_acquire) == atomic_load_explicit(&buffer->next_write, memory_order_acquire);
}

bool coalescingringbuffer_is_full(CoalescingRingBuffer *buffer) { return coalescingringbuffer_size(buffer) == buffer->capacity; }

static inline unsigned int mask(CoalescingRingBuffer *queue, long long value)
{
    return (unsigned int)(value & queue->mask);
}

inline static void coalescingringbuffer_cleanup(CoalescingRingBuffer *buffer)
{
    const unsigned long long last_read = atomic_load_explicit(&buffer->last_read, memory_order_acquire);
    if (buffer->last_cleaned == last_read)
        return;

    while (buffer->last_cleaned < last_read)
    {
        unsigned int index = mask(buffer, ++buffer->last_cleaned);
        free(buffer->keys[index]);
        buffer->keys[index] = NULL;
        void *old = (void *)atomic_exchange_explicit(&buffer->values[index], NULL, memory_order_release);
        if (old != NULL)
            free(old);
    }
}

inline static void coalescingringbuffer_store(CoalescingRingBuffer *buffer, void *key, void *value)
{
    unsigned long long next_write = atomic_load_explicit(&buffer->next_write, memory_order_consume);
    const unsigned int index = mask(buffer, next_write);

    buffer->keys[index] = key;
    atomic_store_explicit(&buffer->values[index], value, memory_order_release);
    atomic_store_explicit(&buffer->next_write, next_write + 1, memory_order_release);
}

inline static bool coalescingringbuffer_add(CoalescingRingBuffer *buffer, void *key, void *value)
{
    if (coalescingringbuffer_is_full(buffer))
    {
        atomic_fetch_add_explicit(&buffer->rejection_count, 1, memory_order_release);
        return false;
    }
    coalescingringbuffer_cleanup(buffer);
    coalescingringbuffer_store(buffer, key, value);
    return true;
}

bool coalescingringbuffer_offer_key(CoalescingRingBuffer *buffer, void *key, void *value)
{
    const unsigned long long next_write = atomic_load_explicit(&buffer->next_write, memory_order_acquire);

    for (unsigned long long update_position = atomic_load_explicit(&buffer->first_write, memory_order_acquire); update_position < next_write; ++update_position)
    {
        const unsigned int index = mask(buffer, update_position);
        if (buffer->key_equals(key, buffer->keys[index]))
        {
            atomic_store_explicit(&buffer->values[index], value, memory_order_release);
            if (update_position >= atomic_load_explicit(&buffer->first_write, memory_order_acquire))
            {
                return true;
            }
            else
            {
                break;
            }
        }
    }
    return coalescingringbuffer_add(buffer, key, value);
}

unsigned int coalescingringbuffer_poll(CoalescingRingBuffer *buffer, unsigned int limit, void (*func)(void *, void *), void *context)
{
    const unsigned long long claim_up_to = min(atomic_load_explicit(&buffer->first_write, memory_order_consume) + limit, atomic_load_explicit(&buffer->next_write, memory_order_acquire));
    atomic_store_explicit(&buffer->first_write, claim_up_to, memory_order_release);

    const unsigned long long last_read = atomic_load_explicit(&buffer->last_read, memory_order_consume);
    for (unsigned long long read_index = last_read + 1; read_index < claim_up_to; ++read_index)
    {
        unsigned int index = mask(buffer, read_index);
        func(context, (void *)atomic_load_explicit(&buffer->values[index], memory_order_acquire));
        void *old = (void *)atomic_exchange_explicit(&buffer->values[index], NULL, memory_order_release);
        if (old == NULL)
            free(old);
    }

    atomic_store_explicit(&buffer->last_read, claim_up_to - 1, memory_order_release);

    return (unsigned int)(claim_up_to - last_read - 1);
}
