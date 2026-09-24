#include "job_queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

JobQueue *init_queue(size_t capacity)
{
    if (capacity == 0) {
        printf("capacity must be greater than 0\n");
        return NULL;
    }

    if (capacity > SIZE_MAX / sizeof(Job)) {
        printf("capacity overflow\n");
        return NULL;
    }

    JobQueue *queue = malloc(sizeof(*queue));
    if (queue == NULL) {
        perror("malloc");
        return NULL;
    }

    queue->jobs = malloc(capacity * sizeof(Job));
    if (queue->jobs == NULL) {
        perror("malloc");
        free(queue);
        return NULL;
    }

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        printf("mutex init failed\n");
        free(queue->jobs);
        free(queue);
        return NULL;
    }

    if (sem_init(&queue->empty_slots, 0, capacity) != 0) {
        printf("semaphore init failed\n");
        pthread_mutex_destroy(&queue->mutex);
        free(queue->jobs);
        free(queue);
        return NULL;
    }

    if (sem_init(&queue->full_slots, 0, 0) != 0) {
        printf("semaphore init failed\n");
        sem_destroy(&queue->empty_slots);
        pthread_mutex_destroy(&queue->mutex);
        free(queue->jobs);
        free(queue);
        return NULL;
    }

    queue->capacity = capacity;
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;

    return queue;
}

int push_job(JobQueue *queue, const char *file_path)
// Adds a new job to the end of the queue.
{

    if (queue == NULL || file_path == NULL) {
        return -1;
    }

    size_t len = strlen(file_path);

    char *path_copy = malloc(len + 1);

    if (path_copy == NULL) {
        perror("malloc");
        return -1;
    }

    strcpy(path_copy, file_path);

    sem_wait(&queue->empty_slots);
    pthread_mutex_lock(&queue->mutex);

    /* Check whether the queue became full. */
    if (queue->count >= queue->capacity) {

        pthread_mutex_unlock(&queue->mutex);

        free(path_copy);

        return -1;
    }

    Job *job = &queue->jobs[queue->tail];
    job->file_path = path_copy;
    job->shutdown = false;

    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->full_slots);

    return 0;
}

int push_shutdown(JobQueue *queue)
{
    if (queue == NULL) {
        return -1;
    }

    sem_wait(&queue->empty_slots);
    pthread_mutex_lock(&queue->mutex);

    Job *job = &queue->jobs[queue->tail];
    job->file_path = NULL;
    job->shutdown = true;

    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->full_slots);

    return 0;
}

Job pop_job(JobQueue *queue)
// Removes and returns the oldest / head job.
{
    if (queue == NULL) {
        return (Job){ .file_path = NULL };
    }

    sem_wait(&queue->full_slots);
    pthread_mutex_lock(&queue->mutex);
   
    if (queue->count == 0) {

        pthread_mutex_unlock(&queue->mutex);

        return (Job){ .file_path = NULL };
    }

    /*
     * Copy the Job out of the queue.
     *
     * We transfer ownership of file_path
     * to the caller.
     */
    Job job = queue->jobs[queue->head];

     /*
     * Clear the queue slot so it no longer
     * points to the transferred string.
     */
    queue->jobs[queue->head].file_path = NULL;

    /*Move head*/
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->empty_slots);

    return job;
}

void destroy_queue(JobQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    size_t index = queue->head;

    for (size_t i = 0; i < queue->count; i++) {
        free(queue->jobs[index].file_path);
        index = (index + 1) % queue->capacity;
    }

    pthread_mutex_unlock(&queue->mutex);

    pthread_mutex_destroy(&queue->mutex);

    free(queue->jobs);
    free(queue);
}

bool validate_job(const Job *job)
{
    if (job == NULL) {
        return false;
    }

    if (job->shutdown) {
        return job->file_path == NULL;
    }

    return job->file_path != NULL && *job->file_path != '\0';
}

bool validate_queue(const JobQueue *queue)
{
    if (queue == NULL) {
        return false;
    }

    if (queue->capacity == 0) {
        return false;
    }

    if (queue->jobs == NULL) {
        return false;
    }

    if (queue->count > queue->capacity) {
        return false;
    }

    if (queue->head >= queue->capacity) {
        return false;
    }

    if (queue->tail >= queue->capacity) {
        return false;
    }

    /* With count-based circular queue, this must hold. */
    if (queue->tail !=
        (queue->head + queue->count) % queue->capacity) {
        return false;
    }

    for (size_t i = 0; i < queue->count; i++) {
        size_t index = (queue->head + i) % queue->capacity;

        if (!validate_job(&queue->jobs[index])) {
            return false;
        }
    }

    return true;
}
