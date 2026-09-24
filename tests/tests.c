#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <test_helpers.h>
#include <job_queue.h>
#include "test_queue.c"

static void test_init(void);
static void test_push_pop(void);
static void test_fifo(void);
static void test_wraparound(void);
static void test_full_buffer_blocking(void);
static void test_empty_buffer_blocking(void);

int main(void)
{
    test_init();
    test_push_pop();
    test_fifo();
    test_wraparound();
    test_full_buffer_blocking();
    test_empty_buffer_blocking();

    printf("All tests passed!\n");

    return 0;
}

