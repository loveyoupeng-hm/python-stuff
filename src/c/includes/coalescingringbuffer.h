#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct CoalescingRingBuffer_t
    {
        long long padding1[128];
        volatile atomic_ullong next_write;
        long long padding2[128];
        unsigned long long last_cleaned;
        volatile atomic_ullong rejection_count;
        void **keys;
        volatile void *_Atomic *_Atomic values;
        // void *non_collapsible_key;
        unsigned int mask;
        unsigned int capacity;
        long long padding3[128];
        volatile atomic_ullong first_write;
        long long padding4[128];
        volatile atomic_ullong last_read;
        bool (*key_equals)(void *, void *);

    } CoalescingRingBuffer;

    CoalescingRingBuffer *coalescingringbuffer_new(unsigned int capacity, bool (*key_equals)(void *, void *));
    unsigned int coalescingringbuffer_size(CoalescingRingBuffer *buffer);
    unsigned int coalescingringbuffer_capacity(CoalescingRingBuffer *buffer);
    unsigned long long coalescingringbuffer_rejection_count(CoalescingRingBuffer *buffer);
    bool coalescingringbuffer_is_empty(CoalescingRingBuffer *buffer);
    bool coalescingringbuffer_is_full(CoalescingRingBuffer *buffer);
    // inline bool coalescingringbuffer_offer_no_key(CoalescingRingBuffer *buffer, void *value)
    // {
    //     return coalescingringbuffer_offer_key(buffer, buffer->non_collapsible_key, value);
    // };
    bool coalescingringbuffer_offer_key(CoalescingRingBuffer *buffer, void *key, void *value);
    unsigned int coalescingringbuffer_poll(CoalescingRingBuffer *buffer, unsigned int limit, void (*func)(void *, void *), void *context);

    inline bool same(void *a, void *b)
    {
        return a == b;
    }

    inline bool key_int_equals(void *a, void *b)
    {
        return same(a, b) || *(int *)a == *(int *)b;
    }

    inline bool key_str_equals(void *a, void *b)
    {
        char *a_str = (char *)a;
        char *b_str = (char *)b;
        return same(a, b) || (strlen(a_str) == strlen(b_str) && strcmp(a_str, b_str) == 0);
    }

#ifdef __cplusplus
}
#endif
