#include <stdio.h>
#include <stdlib.h>

#include "index.h"
#include "job_queue.h"
#include "scanner.h"
#include "test_helpers.h"
#include "worker.h"

void test_scanner_invalid_directory(void)
{
    const char *test_name = "test_scanner_invalid_directory";

    Index *idx = init_index();

    assert_condition(
        idx != NULL,
        test_name,
        "index should be created"
    );

    JobQueue *queue = init_queue(2);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    Scanner scanner = {
        .queue = queue,
        .dir_path = "./fail"
    };

    assert_condition(
        scanner_start(&scanner) == 0,
        test_name,
        "scanner thread should start"
    );

    assert_condition(
        scanner_join(&scanner) == 0,
        test_name,
        "scanner thread should finish"
    );

    assert_condition(
        validate_queue(queue),
        test_name,
        "queue should remain valid"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );

    assert_condition(
        validate_index(idx),
        test_name,
        "index should remain valid"
    );

    assert_condition(
        find_index_entry(idx, "memory") == NULL,
        test_name,
        "index should remain empty"
    );

    destroy_index(idx);
    destroy_queue(queue);
}

void test_scanner_empty_directory(void)
{
    const char *test_name = "test_scanner_empty_directory";

    Index *idx = init_index();

    assert_condition(
        idx != NULL,
        test_name,
        "index should be created"
    );

    JobQueue *queue = init_queue(2);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    Scanner scanner = {
        .queue = queue,
        .dir_path = "./tests/empty"
    };

    assert_condition(
        scanner_start(&scanner) == 0,
        test_name,
        "scanner should start"
    );

    worker_start(queue, idx);

    assert_condition(
        scanner_join(&scanner) == 0,
        test_name,
        "scanner should finish"
    );

    assert_condition(
        validate_queue(queue),
        test_name,
        "queue should remain valid"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );

    assert_condition(
        validate_index(idx),
        test_name,
        "index should remain valid"
    );

    assert_condition(
        find_index_entry(idx, "memory") == NULL,
        test_name,
        "index should remain empty"
    );

    destroy_index(idx);
    destroy_queue(queue);
}

void test_scanner_many_files(void)
{
    const char *test_name = "test_scanner_many_files";
    Index *idx = init_index();

    assert_condition(
        idx != NULL,
        test_name,
        "index should be created"
    );

    JobQueue *queue = init_queue(2);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    Scanner scanner = {
        .queue = queue,
        .dir_path = "./tests/stress"
    };

    assert_condition(
        scanner_start(&scanner) == 0,
        test_name,
        "scanner should start"
    );

    worker_start(queue, idx);

    assert_condition(
        scanner_join(&scanner) == 0,
        test_name,
        "scanner should finish"
    );

    assert_condition(
        validate_queue(queue),
        test_name,
        "queue should remain valid"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );

    assert_condition(
        validate_index(idx),
        test_name,
        "index should remain valid"
    );

    assert_word_in_file(
        idx,
        "01",
        "./tests/stress/file01.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "02",
        "./tests/stress/file02.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "25",
        "./tests/stress/file25.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "50",
        "./tests/stress/file50.txt",
        test_name
    );

    destroy_index(idx);
    destroy_queue(queue);
}

void test_index_word_counts(void)
{
    const char *test_name = "test_index_word_counts";
    Index *idx = init_index();

    assert_condition(
        idx != NULL,
        test_name,
        "index should be created"
    );

    JobQueue *queue = init_queue(8);

    assert_condition(
        queue != NULL,
        test_name,
        "queue should be created"
    );

    assert_condition(
        push_job(queue, "tests/counts.txt") == 0,
        test_name,
        "counts file should be queued"
    );

    for (size_t i = 0; i < WORKER_COUNT; i++) {
        assert_condition(
            push_shutdown(queue) == 0,
            test_name,
            "shutdown job should be queued"
        );
    }

    worker_start(queue, idx);

    assert_word_count(idx, "memory", "tests/counts.txt", 3, test_name);
    assert_word_count(idx, "thread", "tests/counts.txt", 2, test_name);
    assert_word_count(idx, "pointer", "tests/counts.txt", 1, test_name);

    assert_condition(
        validate_index(idx),
        test_name,
        "index should remain valid"
    );

    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );

    destroy_index(idx);
    destroy_queue(queue);
}

int main(void)
{
    test_scanner_invalid_directory();
    test_scanner_empty_directory();
    test_scanner_many_files();
    test_index_word_counts();

    printf("All failure tests passed!\n");
    return EXIT_SUCCESS;
}
