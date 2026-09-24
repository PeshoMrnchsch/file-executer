#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char *file_path;
    bool shutdown;
} Job;

typedef struct {
    Job *jobs;
    size_t capacity;
    size_t count;
    size_t head;
    size_t tail;

    pthread_mutex_t mutex;
    sem_t empty_slots;
    sem_t full_slots;
} JobQueue;



JobQueue *init_queue(size_t capacity);
int push_job(JobQueue *queue, const char *file_path);
int push_shutdown(JobQueue *queue);

Job pop_job(JobQueue *queue);
void destroy_queue(JobQueue *queue);
bool validate_job(const Job *job);
bool validate_queue(const JobQueue *queue);

#endif /* JOB_QUEUE_H */