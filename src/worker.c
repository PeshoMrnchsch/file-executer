#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "worker.h"
#include "file_scanner.h"

// Function executed by each worker thread
void *worker_thread(void *arg)
{
    // Get the Worker context passed by pthread_create()
    Worker *worker = arg;

    // Process jobs
    while (1) {

        // Get the next job from the queue
        Job job = pop_job(worker->queue);

        if (job.shutdown) {
            break;
        }

        // Process the file - no NULL checker - semaphores handle it
        read_file_contents(worker->idx, job.file_path);
        

        // The worker owns the path after pop_job()
        free(job.file_path);
    }

    return NULL;
}

// Create and start all worker threads
void worker_start(JobQueue *queue, Index *idx)
{
    if (queue == NULL || idx == NULL) {
        printf("ERROR: Queue or Index not valid\n");
        return;
    }

    // Allocate one Worker context for each thread
    Worker *workers = malloc(WORKER_COUNT * sizeof(Worker));

    if (workers == NULL) {
        perror("malloc");
        return;
    }

    // Allocate space for all thread IDs
    pthread_t *threads = malloc(WORKER_COUNT * sizeof(*threads));

    if (threads == NULL) {
        perror("malloc");
        free(workers);
        return;
    }

    // Give every worker access to the same queue and index
    for (size_t i = 0; i < WORKER_COUNT; i++) {
        workers[i].queue = queue;
        workers[i].idx = idx;
    }

    // Create all worker threads
    for (size_t i = 0; i < WORKER_COUNT; i++) {

        int res = pthread_create(
            &threads[i],
            NULL,
            worker_thread,
            &workers[i]
        );

        // Handle thread creation failure
        if (res != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(res));

            // Wait for threads that were already created
            for (size_t j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            free(threads);
            free(workers);
            return;
        }
    }

    // Wait for every worker thread to finish
    for (size_t i = 0; i < WORKER_COUNT; i++) {

        int res = pthread_join(threads[i], NULL);

        if (res != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(res));
        }
    }

    // Free thread and worker arrays
    free(threads);
    free(workers);
}