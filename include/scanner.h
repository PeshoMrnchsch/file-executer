#ifndef SCANNER_H
#define SCANNER_H

#include <pthread.h>

#include "job_queue.h"

typedef struct {
	JobQueue *queue;
	const char *dir_path;
	pthread_t thread;
} Scanner;

void *scanner_thread(void *arg);
int scanner_start(Scanner *scanner);
int scanner_join(Scanner *scanner);

#endif /* SCANNER_H */
