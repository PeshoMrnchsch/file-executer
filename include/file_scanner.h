#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <stddef.h>

#include "index.h"
#include "job_queue.h"

int has_txt_extension(const char *filename);
int make_file_path(const char *dir_path,
                   const char *filename,
                   char *output_path,
                   size_t output_size);
void read_file_contents(Index *idx, const char *file_path);
int list_txt_files(JobQueue *queue, const char *dir_path);

#endif /* FILE_SCANNER_H */
