#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <worker.h>
#include <index.h>
#include <job_queue.h>
#include <test_helpers.h>

static const size_t worker_count = WORKER_COUNT;

static void push_shutdown_jobs(JobQueue *queue)
{
    for (size_t i = 0; i < worker_count; i++) {
        assert_condition(
            push_shutdown(queue) == 0,
            "shutdown",
            "shutdown job should be queued"
        );
    }
}

static void finish_workers(JobQueue *queue, Index *idx)
{
    push_shutdown_jobs(queue);
    worker_start(queue, idx);
}

static void test_worker_single_job(void)
{
    const char *test_name = "test_worker_single_job";
    JobQueue *queue = init_queue(24);
    Index *idx = init_index();

    assert_condition(queue != NULL, test_name, "queue should be created");
    assert_condition(idx != NULL, test_name, "index should be created");
    assert_condition(push_job(queue, "tests/medium1.txt") == 0,
                     test_name,
                     "job should be queued");

    finish_workers(queue, idx);

    assert_word_in_file(idx, "memory", "tests/medium1.txt", test_name);
    assert_condition(queue->count == 0,
                     test_name,
                     "queue should be empty after workers finish");
    assert_condition(validate_index(idx), test_name, "index should be valid");

    destroy_index(idx);
    destroy_queue(queue);
}

static void test_worker_multiple_jobs(void)
{
    const char *test_name = "test_worker_multiple_jobs";
    const char *files[] = {
        "tests/medium1.txt",
        "tests/medium2.txt",
        "tests/medium3.txt",
        "tests/medium4.txt"
    };
    JobQueue *queue = init_queue(8);
    Index *idx = init_index();

    assert_condition(queue != NULL, test_name, "queue should be created");
    assert_condition(idx != NULL, test_name, "index should be created");

    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
        assert_condition(push_job(queue, files[i]) == 0,
                         test_name,
                         "job should be queued");
    }

    finish_workers(queue, idx);

    assert_word_in_file(idx, "memory", files[0], test_name);
    assert_word_in_file(idx, "semaphore", files[1], test_name);
    assert_word_in_file(idx, "consumer", files[2], test_name);
    assert_word_in_file(idx, "producer", files[3], test_name);
    assert_condition(queue->count == 0,
                     test_name,
                     "queue should be empty after workers finish");
    assert_condition(validate_index(idx), test_name, "index should be valid");

    destroy_index(idx);
    destroy_queue(queue);
}

static void test_multiple_workers(void)
{
    const char *test_name = "test_multiple_workers";
    JobQueue *queue = init_queue(24);
    Index *idx = init_index();

    assert_condition(queue != NULL, test_name, "queue should be created");
    assert_condition(idx != NULL, test_name, "index should be created");

    for (size_t i = 0; i < 20; i++) {
        const char *file = i % 2 == 0
            ? "tests/medium1.txt"
            : "tests/medium2.txt";
        assert_condition(push_job(queue, file) == 0,
                         test_name,
                         "job should be queued");
    }

    finish_workers(queue, idx);

    assert_word_in_file(idx, "thread", "tests/medium1.txt", test_name);
    assert_word_in_file(idx, "semaphore", "tests/medium2.txt", test_name);
    assert_condition(queue->count == 0,
                     test_name,
                     "shared queue should be empty");
    assert_condition(validate_index(idx), test_name, "shared index should be valid");

    destroy_index(idx);
    destroy_queue(queue);
}

typedef struct {
    JobQueue *queue;
    Index *idx;
} WorkerStartArgs;

static void *worker_start_thread(void *arg)
{
    WorkerStartArgs *args = arg;
    worker_start(args->queue, args->idx);
    return NULL;
}

static void test_worker_empty_queue_blocking(void)
{
    const char *test_name = "test_worker_empty_queue_blocking";
    JobQueue *queue = init_queue(8);
    Index *idx = init_index();
    WorkerStartArgs args = { .queue = queue, .idx = idx };
    pthread_t starter;

    assert_condition(queue != NULL, test_name, "queue should be created");
    assert_condition(idx != NULL, test_name, "index should be created");

    create_thread_or_fail(&starter,
                          worker_start_thread,
                          &args,
                          test_name);
    sleep_briefly();

    assert_condition(queue->count == 0,
                     test_name,
                     "workers should wait while queue is empty");

    assert_condition(push_job(queue, "tests/medium1.txt") == 0,
                     test_name,
                     "job should wake a worker");
    push_shutdown_jobs(queue);
    join_thread_or_fail(starter, test_name);

    assert_word_in_file(idx, "memory", "tests/medium1.txt", test_name);
    assert_condition(queue->count == 0,
                     test_name,
                     "queue should be empty after processing");

    destroy_index(idx);
    destroy_queue(queue);
}

static void test_worker_shutdown(void)
{
    const char *test_name = "test_worker_shutdown";
    JobQueue *queue = init_queue(8);
    Index *idx = init_index();

    assert_condition(queue != NULL, test_name, "queue should be created");
    assert_condition(idx != NULL, test_name, "index should be created");
    assert_condition(push_job(queue, "tests/medium3.txt") == 0,
                     test_name,
                     "job should be queued");

    finish_workers(queue, idx);

    assert_word_in_file(idx, "consumer", "tests/medium3.txt", test_name);
    assert_condition(queue->count == 0,
                     test_name,
                     "all workers should stop after shutdown");

    destroy_index(idx);
    destroy_queue(queue);
}

int main(void)
{
    test_worker_single_job();
    test_worker_multiple_jobs();
    test_multiple_workers();
    test_worker_empty_queue_blocking();
    test_worker_shutdown();

    printf("All worker tests passed!\n");
    return 0;
}