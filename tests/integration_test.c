#include <stdio.h>
#include <stdlib.h>

#include "index.h"
#include "job_queue.h"
#include "scanner.h"
#include "worker.h"
#include "test_helpers.h"


static void test_scanner_workers_integration(void)
{
    char *test_name = "test_scanner_workers_integration";

    Index *idx = init_index();
    if (idx == NULL) {
        printf("FAILED: could not create index\n");
        return;
    }

    JobQueue *queue = init_queue(2);
    if (queue == NULL) {
        printf("FAILED: could not create queue\n");
        destroy_index(idx);
        return;
    }
    Scanner scanner = {.queue = queue, .dir_path = "./tests"};
    
    assert_condition(
        scanner_start(&scanner) == 0,
        test_name,
        "scanner should start"
    );
    worker_start(queue, idx);

    assert_condition(
        scanner_join(&scanner) == 0,
        test_name,
        "scanner should finish successfully"
    );

    
    
    // Verify actual results
    assert_word_in_file(
        idx,
        "memory",
        "./tests/medium1.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "semaphore",
        "./tests/medium2.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "consumer",
        "./tests/medium3.txt",
        test_name
    );

    assert_word_in_file(
        idx,
        "producer",
        "./tests/medium4.txt",
        test_name
    );

    // Verify data structure
    assert_condition(
        validate_index(idx),
        test_name,
        "index should be valid"
    );

    // Verify queue
    assert_condition(
        queue->count == 0,
        test_name,
        "queue should be empty"
    );

    destroy_index(idx);
    destroy_queue(queue);

}


int main(void) { 
    test_scanner_workers_integration();
    printf("All scanner-worker integration tests passed!\n");
    return EXIT_SUCCESS;
}