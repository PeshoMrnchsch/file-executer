#define _POSIX_C_SOURCE 200809L

#include "test_helpers.h"
#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void assert_condition(int condition,
                      const char *test_name,
                      const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s - %s\n", test_name, message);
        exit(EXIT_FAILURE);
    }
}

void sleep_briefly(void)
{
    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 100000000
    };

    nanosleep(&delay, NULL);
}

void create_thread_or_fail(pthread_t *thread,
                           void *(*thread_function)(void *),
                           void *arg,
                           const char *test_name)
{
    int result = pthread_create(thread, NULL, thread_function, arg);

    if (result != 0) {
        fprintf(stderr,
                "FAIL: %s - pthread_create failed: %s\n",
                test_name,
                strerror(result));

        exit(EXIT_FAILURE);
    }
}

void join_thread_or_fail(pthread_t thread,
                         const char *test_name)
{
    int result = pthread_join(thread, NULL);

    if (result != 0) {
        fprintf(stderr,
                "FAIL: %s - pthread_join failed: %s\n",
                test_name,
                strerror(result));

        exit(EXIT_FAILURE);
    }
}

void assert_queue_empty(const JobQueue *queue,
                        const char *test_name)
{
    assert_condition(
        queue != NULL,
        test_name,
        "queue is NULL"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );
}

void assert_queue_full(const JobQueue *queue,
                       const char *test_name)
{
    assert_condition(
        queue != NULL,
        test_name,
        "queue is NULL"
    );

    assert_condition(
        queue->count == queue->capacity,
        test_name,
        "queue should be full"
    );
}

void assert_word_in_file(Index *idx,
                                const char *word,
                                const char *filename,
                                const char *test_name)
{
    IndexEntry *entry = find_index_entry(idx, word);

    assert_condition(entry != NULL, test_name, "word should be indexed");

    for (size_t i = 0; i < entry->file_count; i++) {
        if (strcmp(entry->files[i].filename, filename) == 0) {
            assert_condition(entry->files[i].wordcount > 0,
                             test_name,
                             "indexed word count should be positive");
            return;
        }
    }

    assert_condition(0, test_name, "word should be indexed for the file");
}

void assert_word_count(Index *idx,
                       const char *word,
                       const char *filename,
                       size_t expected_count,
                       const char *test_name)
{
    IndexEntry *entry = find_index_entry(idx, word);

    assert_condition(entry != NULL, test_name, "word should be indexed");

    for (size_t i = 0; i < entry->file_count; i++) {
        if (strcmp(entry->files[i].filename, filename) == 0) {
            assert_condition(
                entry->files[i].wordcount == expected_count,
                test_name,
                "word count did not match expected value"
            );
            return;
        }
    }

    assert_condition(0, test_name, "word should be indexed for the file");
}