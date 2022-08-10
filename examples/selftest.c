/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2022 Sanpe <sanpeqf@gmail.com>
 */

#include "range.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * ARRAY_SIZE - get the number of elements in array.
 * @arr: array to be sized.
 */
#define ARRAY_SIZE(arr) ( \
    sizeof(arr) / sizeof((arr)[0]) \
)

struct range_test_entry {
    unsigned long start;
    unsigned long size;
    unsigned int type;
};

struct range_test_entry range_test_table[] = {
    { 0x80000000, 0x40000000, 0 }, /* 0: base range */
    { 0x80000000, 0x20000000, 1 }, /* 1: inside front cover */
    { 0xa0000000, 0x20000000, 1 }, /* 2: inside back cover */
    { 0x70000000, 0x20000000, 2 }, /* 3: outside front cover */
    { 0xb0000000, 0x20000000, 3 }, /* 4: outside back cover */
    { 0x78000000, 0x50000000, 4 }, /* 5: completely cover */
    { 0x90000000, 0x20000000, 5 }, /* 6: middle cover */
};

static struct range_node *range_test_alloc(void *pdata)
{
    return malloc(sizeof(struct range_node));
}

static void range_test_free(struct range_node *node, void *pdata)
{
    free(node);
}

static void range_test_dump(struct range_head *head)
{
    struct range_node *node;
    unsigned long end;

    list_for_each_entry(node, &head->nodes, list) {
        end = node->start + node->size - 1;
        printf("  [%#010lx - %#010lx] %d\n",
               node->start, end, node->type);
    }
}

int main(void)
{
    DEFINE_RANGE(range_test, range_test_alloc, range_test_free, NULL, NULL);
    unsigned int count;

    for (count = 0; count < ARRAY_SIZE(range_test_table); ++count) {
        struct range_test_entry *entry = range_test_table + count;
        range_insert(&range_test, entry->type, entry->start, entry->size, RANGE_DISLODGE, true);
        printf("range dislodge test%u: %d\n", count, errno);
        if (errno) {
            range_release(&range_test);
            return errno;
        }
        range_test_dump(&range_test);
    }
    range_release(&range_test);

    for (count = 0; count < ARRAY_SIZE(range_test_table); ++count) {
        struct range_test_entry *entry = range_test_table + count;
        range_insert(&range_test, entry->type, entry->start, entry->size, RANGE_RETRACTION, true);
        printf("range retraction test%u: %d\n", count, errno);
        if (errno && errno != -ENODATA) {
            range_release(&range_test);
            return errno;
        }
        range_test_dump(&range_test);
    }
    range_release(&range_test);

    return 0;
}
