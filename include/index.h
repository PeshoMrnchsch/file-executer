#ifndef INDEX_H
#define INDEX_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>


#define BUCKET_COUNT 101
#define HASH_VALUE 5381
#define INITIAL_FILE_CAPACITY 4

typedef struct {
    char *filename;
    size_t wordcount;  // Occurrences of word in this file
} FileInfo;

typedef struct IndexEntry {
    char *word;

    FileInfo *files;
    size_t file_count;       // Current number of FileInfo items
    size_t file_capacity;    // Allocated FileInfo capacity

    struct IndexEntry *next; // Next entry in the bucket
} IndexEntry;

typedef struct {
    size_t bucket_count;
    IndexEntry **buckets;
    pthread_mutex_t mutex;
} Index;


/* Initialization / cleanup */
Index *init_index(void);
void destroy_index(Index *idx);


/* Hashing */
unsigned long hash_word(const char *word);


/* Index operations */
int add_word_to_index(Index *idx,
                      const char *word,
                      const char *filename);

IndexEntry *find_index_entry(const Index *idx,
                             const char *word);

void print_index(const Index *idx);

void run_index_tests(Index *idx);


/* Dynamic FileInfo storage */
int ensure_file_capacity(IndexEntry *entry);

bool validate_index(const Index *idx);

#endif /* INDEX_H */