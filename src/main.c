// #include <stdlib.h>
// #include <stdio.h>

// #include "file_scanner.h"
// #include "scanner.h"
// #include "index.h"
// #include "job_queue.h"

// int main(void)
// {
// 	Index *idx = init_index();

// 	if (idx == NULL) {
// 		fprintf(stderr, "Failed to initialize index\n");
// 		return EXIT_FAILURE;
// 	}

// 	JobQueue *queue = init_queue(1024);

// 	if (queue == NULL) {
// 		fprintf(stderr, "Failed to initialize job queue\n");
// 		destroy_index(idx);
// 		return EXIT_FAILURE;
// 	}

//     pthread_t scanner_thread;

//     pthread_create(&scanner_thread, NULL, scanner_worker, &scanner);
    
// 	list_txt_files(queue, "./test_data");
// 	worker_start(queue, idx);

// 	print_index(idx);

// 	destroy_queue(queue);
// 	destroy_index(idx);

// 	return EXIT_SUCCESS;
// }