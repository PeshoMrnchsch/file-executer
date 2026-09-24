#define _POSIX_C_SOURCE 200809L

#include "file_scanner.h"
#include "scanner.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "tokenizer.h"
#include "job_queue.h"
#include "worker.h"

#define MAX_LINE_LENGTH 256

int has_txt_extension(const char *filename)
{
    if (filename == NULL) {
        return 0;
    }

    size_t len = strlen(filename);
    if (len < 4) {
        return 0;
    }

    return strcmp(filename + len - 4, ".txt") == 0;
}

int make_file_path(const char *dir_path,
                   const char *filename,
                   char *output_path,
                   size_t output_size)
{
    if (dir_path == NULL || filename == NULL || output_path == NULL ||
        output_size == 0) {
        return -1;
    }

    int written = snprintf(
        output_path,
        output_size,
        "%s/%s",
        dir_path,
        filename
    );

    return written < 0 || (size_t)written >= output_size ? -1 : 0;
}

void read_file_contents(Index *idx, const char *file_path)
{
    if (idx == NULL || file_path == NULL) {
        return;
    }

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return;
    }
    
    const char *delimiters = " \t\n\r";

    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char *saveptr = NULL;
        char *word = strtok_r(buffer, delimiters, &saveptr);
        while (word != NULL) {

            clean_and_normalize(word);
            
            if(*word != '\0'){
                pthread_mutex_lock(&idx->mutex);
                int added = add_word_to_index(idx, word, file_path);
                if(added == 0){
                    fprintf(stderr, "Failed to add word to index\n");
                }
                pthread_mutex_unlock(&idx->mutex);

            }

            word = strtok_r(NULL, delimiters, &saveptr);
        }
    }

    fclose(file);
}

/*
Scans a specified directory for files with a .txt extension
*/
int list_txt_files(JobQueue *queue, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return -1;
    }

    struct dirent *entry;
    char full_path[512];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (has_txt_extension(entry->d_name)) {
            if (make_file_path(
                dir_path,
                entry->d_name,
                full_path,
                sizeof(full_path)
            ) != 0) {
                fprintf(stderr, "File path is too long: %s/%s\n",
                        dir_path, entry->d_name);
                closedir(dir);
                return -1;
            }

            if (push_job(queue, full_path) != 0) {
                fprintf(stderr, "Failed to push job into JobQueue\n");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return 0;
}

void *scanner_thread(void *arg)
{
    Scanner *scanner = arg;

    // Pushes the files into queue
    if (list_txt_files(scanner->queue, scanner->dir_path) != 0) {
        fprintf(stderr, "Failed to list text files\n");
        return NULL;
    }
    
    // Initalize the empty jobs since there are no more files to read
    for (size_t i = 0; i < WORKER_COUNT; i++) {
        if (push_shutdown(scanner->queue) != 0) {
            fprintf(stderr, "Failed to enqueue shutdown job\n");
            return NULL;
        }
    }
    
    return NULL;
}

/* Start the producer and let the caller continue.*/
int scanner_start(Scanner *scanner){
    if (
        scanner == NULL || 
        scanner->queue == NULL ||
        scanner->dir_path == NULL){
        return -1; 
    }

    if (pthread_create(&scanner->thread, NULL, scanner_thread, scanner) != 0)
    {
        fprintf(stderr, "Failed to create scanner thread\n");
        return -1;
    } 

    return 0;
}

int scanner_join(Scanner *scanner)
{
    if (scanner == NULL) return -1;

    int result = pthread_join(scanner->thread, NULL);
    if (result != 0) {
        fprintf(stderr, "Failed to join scanner thread\n");
        return -1;
    }

    return 0;
}
