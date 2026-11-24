/*
 * solver.c - Infer the geometry and policy of the lab's simulated cache.
 */
#include "solver.h"

/* Any block absent after the fill causes a miss before it can be restored. */
static int all_fit(uint32_t stride, int count)
{
    cache_flush();
    for (int i = 0; i < count; ++i)
        cache_load(stride * i);
    for (int i = 0; i < count; ++i) {
        if (!cache_load(stride * i))
            return 0;
    }
    return 1;
}

void solve(void)
{
    int b = 0;

    cache_flush();
    cache_load(0);
    while (cache_load(1u << b))
        ++b;

    /* A stride covering the largest geometry maps every block to set zero. */
    uint32_t stride = 1u << (b + 9);
    int E = 1;
    while (E < 16 && all_fit(stride, E + 1))
        ++E;

    /* The first stride that cannot hold E + 1 blocks uses exactly s set bits. */
    int s = 1;
    while (s < 9 && all_fit(1u << (b + s), E + 1))
        ++s;

    cache_flush();
    cache_store(0);
    int write_allocate = cache_load(0);

    int policy = POLICY_LRU;
    if (E > 1) {
        int saw_hit = 0;
        int saw_miss = 0;
        for (int trial = 0; trial < 512 && !(saw_hit && saw_miss); ++trial) {
            cache_flush();
            for (int i = 0; i < E; ++i)
                cache_load(stride * i);
            cache_load(0);
            cache_load(stride * E);

            /* FIFO retains line 1, LRU evicts it, RANDOM can do either. */
            if (cache_load(stride))
                saw_hit = 1;
            else
                saw_miss = 1;
        }
        if (saw_hit)
            policy = saw_miss ? POLICY_RANDOM : POLICY_FIFO;
    }
    report(b, s, E, policy, write_allocate);
}
