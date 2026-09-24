#include<stdbool.h>
#include<stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>


#include<test_helpers.h>
#include<job_queue.h>
#include<worker.h>
#include<scanner.h>

static void test_init(void)
{
    const char *test_name = "test_init";
    const size_t capacity = 4;

    JobQueue *queue = init_queue(capacity);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    assert_condition(
        queue->capacity == capacity,
        test_name,
        "capacity should be initialized correctly"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "count should start at zero"
    );

    assert_condition(
        queue->head == 0,
        test_name,
        "head should start at zero"
    );

    assert_condition(
        queue->tail == 0,
        test_name,
        "tail should start at zero"
    );

    assert_condition(
        validate_queue(queue),
        test_name,
        "queue validation should succeed"
    );

    destroy_queue(queue);
}

static void test_push_pop(void)
{
    const char *test_name = "test_push_pop";

    JobQueue *queue = init_queue(2);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    const char *path = "test.txt";

    assert_condition(
        push_job(queue, path) == 0,
        test_name,
        "push_job should succeed"
    );

    Job job = pop_job(queue);

    assert_condition(
        job.file_path != NULL,
        test_name,
        "popped job should contain a file path"
    );

    assert_condition(
        strcmp(job.file_path, path) == 0,
        test_name,
        "popped path should match pushed path"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty after pop"
    );

    free(job.file_path);

    destroy_queue(queue);
}

static void test_fifo(void)
{
    const char *test_name = "test_fifo";

    JobQueue *queue = init_queue(3);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    const char *paths[] = {
        "first.txt",
        "second.txt",
        "third.txt"
    };

    for (size_t i = 0; i < 3; i++) {
        assert_condition(
            push_job(queue, paths[i]) == 0,
            test_name,
            "push_job should succeed"
        );
    }

    for (size_t i = 0; i < 3; i++) {
        Job job = pop_job(queue);

        assert_condition(
            job.file_path != NULL,
            test_name,
            "popped job should contain a file path"
        );

        assert_condition(
            strcmp(job.file_path, paths[i]) == 0,
            test_name,
            "jobs should be returned in FIFO order"
        );

        free(job.file_path);
    }

    assert_queue_empty(queue, test_name);

    destroy_queue(queue);
}

static void test_wraparound(void)
{
    const char *test_name = "test_wraparound";

    JobQueue *queue = init_queue(2);

    assert_condition(queue != NULL,
                     test_name,
                     "queue should be created");

    push_job(queue, "first.txt");
    push_job(queue, "second.txt");

    Job job = pop_job(queue);

    assert_condition(strcmp(job.file_path, "first.txt") == 0,
                     test_name,
                     "first job should be returned");

    free(job.file_path);

    push_job(queue, "third.txt");

    job = pop_job(queue);

    assert_condition(strcmp(job.file_path, "second.txt") == 0,
                     test_name,
                     "second job should be returned first");

    free(job.file_path);

    job = pop_job(queue);

    assert_condition(strcmp(job.file_path, "third.txt") == 0,
                     test_name,
                     "third job should be returned second");

    free(job.file_path);

    assert_queue_empty(queue, test_name);

    destroy_queue(queue);
}

static void *producer_thread(void *arg)
{
    JobQueue *queue = arg;

    push_job(queue, "second.txt");

    return NULL;
}

static void test_full_buffer_blocking(void)
{
    const char *test_name = "test_full_buffer_blocking";

    JobQueue *queue = init_queue(1);

    push_job(queue, "first.txt");

    pthread_t producer;
    create_thread_or_fail(
        &producer,
        producer_thread,
        queue,
        test_name
    );

    sleep_briefly();

    Job job = pop_job(queue);
    assert_condition(strcmp(job.file_path, "first.txt") == 0,
                     test_name,
                     "first job should be returned");

    free(job.file_path);

    join_thread_or_fail(producer, test_name);

    job = pop_job(queue);

    assert_condition(strcmp(job.file_path, "second.txt") == 0,
                     test_name,
                     "second job should be produced after space becomes available");

    
    free(job.file_path);
    assert_queue_empty(queue, test_name);
    destroy_queue(queue);
}


typedef struct {
    JobQueue *queue;
    Job job;
} ConsumerArgs;

static void *consumer_thread(void *arg) 
{
    ConsumerArgs *args = arg;
    args->job = pop_job(args->queue);
    
    return NULL;

}
static void test_empty_buffer_blocking(void)
{
    const char *test_name = "test_empty_buffer_blocking";

    JobQueue *queue = init_queue(1);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    ConsumerArgs args = {
        .queue = queue,
        .job = { .file_path = NULL }
    };

    pthread_t consumer;

    create_thread_or_fail(
        &consumer,
        consumer_thread,
        &args,
        test_name
    );

    sleep_briefly();

    push_job(queue, "test.txt");

    join_thread_or_fail(
        consumer,
        test_name
    );

    assert_condition(
        args.job.file_path != NULL,
        test_name,
        "consumer should receive a job"
    );

    assert_condition(
        strcmp(args.job.file_path, "test.txt") == 0,
        test_name,
        "consumer should receive the correct job"
    );

    free(args.job.file_path);

    assert_queue_empty(queue, test_name);

    destroy_queue(queue);
}