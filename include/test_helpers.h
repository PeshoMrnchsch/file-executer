#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <pthread.h>

#include "job_queue.h"
#include "index.h"

void assert_condition(int condition,
                      const char *test_name,
                      const char *message);

void sleep_briefly(void);

void create_thread_or_fail(pthread_t *thread,
                           void *(*thread_function)(void *),
                           void *arg,
                           const char *test_name);

void join_thread_or_fail(pthread_t thread,
                         const char *test_name);

void assert_queue_empty(const JobQueue *queue,
                        const char *test_name);

void assert_queue_full(const JobQueue *queue,
                       const char *test_name);

void assert_word_in_file(Index *idx,
                                const char *word,
                                const char *filename,
                                const char *test_name);

void assert_word_count(Index *idx,
                       const char *word,
                       const char *filename,
                       size_t expected_count,
                       const char *test_name);

#endif