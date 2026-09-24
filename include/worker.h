#ifndef WORKER_H
#define WORKER_H

#include "job_queue.h"
#include "index.h"
#define WORKER_COUNT 4


typedef struct 
{
    JobQueue *queue;
    Index *idx;

} Worker;

void worker_start(JobQueue *queue, Index *idx);

#endif /* WORKER_H */
