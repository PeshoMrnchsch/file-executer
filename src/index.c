#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/* ----------------------------------------------------------
 * Helper: duplicate a string
 * ---------------------------------------------------------- */
static char *copy_string(const char *source)
{
    size_t length = strlen(source);

    char *copy = malloc(length + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, source, length + 1);

    return copy;
}


/* ----------------------------------------------------------
 * Ensure there is enough space in an IndexEntry's files array
 * ---------------------------------------------------------- */
int ensure_file_capacity(IndexEntry *entry)
{
    if (entry == NULL) {
        return -1;
    }

    /* There is still free space */
    if (entry->file_count < entry->file_capacity) {
        return 0;
    }

    /* Prevent overflow when doubling */
    if (entry->file_capacity > SIZE_MAX / 2) {
        return -1;
    }

    size_t new_capacity;

    if (entry->file_capacity == 0) {
        new_capacity = INITIAL_FILE_CAPACITY;
    } else {
        new_capacity = entry->file_capacity * 2;
    }

    /* Prevent overflow in new_capacity * sizeof(FileInfo) */
    if (new_capacity > SIZE_MAX / sizeof(FileInfo)) {
        return -1;
    }

    FileInfo *temp = realloc(
        entry->files,
        new_capacity * sizeof(*entry->files)
    );

    if (temp == NULL) {
        return -1;
    }

    entry->files = temp;
    entry->file_capacity = new_capacity;

    return 0;
}


/* ----------------------------------------------------------
 * Initialize index
 * ---------------------------------------------------------- */
Index *init_index(void)
{
    Index *idx = malloc(sizeof(*idx));

    if (idx == NULL) {
        perror("malloc");
        return NULL;
    }

    idx->bucket_count = BUCKET_COUNT;

    idx->buckets = calloc(
        idx->bucket_count,
        sizeof(*idx->buckets)
    );

    if (idx->buckets == NULL) {
        perror("calloc");
        free(idx);
        return NULL;
    }

    if(pthread_mutex_init(&idx->mutex, NULL) != 0){
        perror("mutex");
        free(idx->buckets);
        free(idx);
        return NULL;
    }

    return idx;
}


/* ----------------------------------------------------------
 * djb2 hash function
 * ---------------------------------------------------------- */
unsigned long hash_word(const char *word)
{
    unsigned long hash = HASH_VALUE;

    while (*word != '\0') {
        hash = hash * 33 + (unsigned char)*word;
        word++;
    }

    return hash;
}


/* ----------------------------------------------------------
 * Add word + filename to index
 * ---------------------------------------------------------- */
int add_word_to_index(
    Index *idx,
    const char *word,
    const char *filename
)
{
    if (idx == NULL ||
        idx->buckets == NULL ||
        idx->bucket_count == 0 ||
        word == NULL ||
        filename == NULL ||
        *word == '\0' ||
        *filename == '\0') {
        return 0;
    }

    /* Find bucket */
    size_t bucket_idx =
        hash_word(word) % idx->bucket_count;

    /* Search linked list in bucket */
    IndexEntry *cur = idx->buckets[bucket_idx];

    while (cur != NULL) {

        /* Word already exists */
        if (strcmp(cur->word, word) == 0) {

            /* Search for filename */
            for (size_t i = 0; i < cur->file_count; i++) {

                if (strcmp(cur->files[i].filename, filename) == 0) {

                    /* Same word + same file */
                    cur->files[i].wordcount++;

                    return 1;
                }
            }

            /* Word exists, but this file does not */
            if (ensure_file_capacity(cur) != 0) {
                return 0;
            }

            char *filename_copy = copy_string(filename);

            if (filename_copy == NULL) {
                return 0;
            }

            cur->files[cur->file_count].filename = filename_copy;
            cur->files[cur->file_count].wordcount = 1;

            cur->file_count++;

            return 1;
        }

        cur = cur->next;
    }


    /* ------------------------------------------------------
     * Word does not exist
     * Create a new IndexEntry
     * ------------------------------------------------------ */

    IndexEntry *new_entry = malloc(sizeof(*new_entry));

    if (new_entry == NULL) {
        perror("malloc");
        return 0;
    }

    /* Copy word */
    new_entry->word = copy_string(word);

    if (new_entry->word == NULL) {
        free(new_entry);
        return 0;
    }

    /* Initial file array */
    new_entry->file_capacity = INITIAL_FILE_CAPACITY;
    new_entry->file_count = 0;

    if (new_entry->file_capacity >
        SIZE_MAX / sizeof(*new_entry->files)) {

        free(new_entry->word);
        free(new_entry);

        return 0;
    }

    new_entry->files = malloc(
        new_entry->file_capacity *
        sizeof(*new_entry->files)
    );

    if (new_entry->files == NULL) {
        perror("malloc");
        free(new_entry->word);
        free(new_entry);

        return 0;
    }

    /* Add first filename */
    new_entry->files[0].filename =
        copy_string(filename);

    if (new_entry->files[0].filename == NULL) {
        free(new_entry->files);
        free(new_entry->word);
        free(new_entry);

        return 0;
    }

    new_entry->files[0].wordcount = 1;
    new_entry->file_count = 1;

    /* Insert at beginning of bucket */
    new_entry->next = idx->buckets[bucket_idx];

    idx->buckets[bucket_idx] = new_entry;

    return 1;
}


/* ----------------------------------------------------------
 * Find a word in the index
 * ---------------------------------------------------------- */
IndexEntry *find_index_entry(
    const Index *idx,
    const char *word
)
{
    if (idx == NULL ||
        idx->buckets == NULL ||
        idx->bucket_count == 0 ||
        word == NULL ||
        *word == '\0') {
        return NULL;
    }

    size_t bucket_idx =
        hash_word(word) % idx->bucket_count;

    IndexEntry *cur = idx->buckets[bucket_idx];

    while (cur != NULL) {

        if (strcmp(cur->word, word) == 0) {
            return cur;
        }

        cur = cur->next;
    }

    return NULL;
}


/* ----------------------------------------------------------
 * Print entire index
 * ---------------------------------------------------------- */
void print_index(const Index *idx)
{
    if (idx == NULL ||
        idx->buckets == NULL) {
        return;
    }

    printf("\n========== INDEX ==========\n");

    for (size_t i = 0; i < idx->bucket_count; i++) {

        const IndexEntry *cur = idx->buckets[i];

        if (cur == NULL) {
            continue;
        }

        printf("Bucket %zu:\n", i);

        while (cur != NULL) {

            printf("  Word: \"%s\"\n", cur->word);

            printf(
                "    Files (%zu/%zu):\n",
                cur->file_count,
                cur->file_capacity
            );

            for (size_t j = 0; j < cur->file_count; j++) {

                printf(
                    "      %s -> %zu occurrence(s)\n",
                    cur->files[j].filename,
                    cur->files[j].wordcount
                );
            }

            cur = cur->next;
        }
    }

    printf("===========================\n\n");
}


/* ----------------------------------------------------------
 * Destroy entire index
 * ---------------------------------------------------------- */
void destroy_index(Index *idx)
{
    if (idx == NULL) {
        return;
    }

    if (idx->buckets != NULL) {

        for (size_t i = 0;
             i < idx->bucket_count;
             i++) {

            IndexEntry *cur = idx->buckets[i];

            while (cur != NULL) {

                IndexEntry *next = cur->next;

                /* Free every filename */
                for (size_t j = 0;
                     j < cur->file_count;
                     j++) {

                    free(cur->files[j].filename);
                }

                /* Free FileInfo array */
                free(cur->files);

                /* Free word */
                free(cur->word);

                /* Free IndexEntry */
                free(cur);

                cur = next;
            }
        }

        /* Free bucket array */
        free(idx->buckets);
    }

     /* Free Mutex */
    pthread_mutex_destroy(&idx->mutex);
    
    /* Free Index itself */
    free(idx);
}

void run_index_tests(Index *idx)
{
    printf("\n========== TESTS ==========\n");

    add_word_to_index(idx, "memory", "a.txt");
    add_word_to_index(idx, "memory", "a.txt");
    add_word_to_index(idx, "memory", "a.txt");

    IndexEntry *entry = find_index_entry(idx, "memory");

    if (entry == NULL) {
        printf("TEST 1 FAILED: memory not found\n");
    }
    else if (entry->file_count != 1 ||
             entry->files[0].wordcount != 3) {
        printf("TEST 1 FAILED: wrong count\n");
    }
    else {
        printf("TEST 1 PASSED\n");
    }

    add_word_to_index(idx, "memory", "b.txt");
    add_word_to_index(idx, "memory", "c.txt");

    entry = find_index_entry(idx, "memory");

    if (entry == NULL) {
        printf("TEST 2 FAILED: memory not found\n");
    }
    else if (entry->file_count != 3) {
        printf("TEST 2 FAILED: expected 3 files, got %zu\n",
               entry->file_count);
    }
    else {
        printf("TEST 2 PASSED\n");
    }

    add_word_to_index(idx, "thread", "a.txt");

    entry = find_index_entry(idx, "thread");

    if (entry == NULL) {
        printf("TEST 3 FAILED: thread not found\n");
    }
    else if (entry->file_count != 1 ||
             entry->files[0].wordcount != 1) {
        printf("TEST 3 FAILED\n");
    }
    else {
        printf("TEST 3 PASSED\n");
    }

    entry = find_index_entry(idx, "nonexistent");

    if (entry != NULL) {
        printf("TEST 4 FAILED: nonexistent word found\n");
    }
    else {
        printf("TEST 4 PASSED\n");
    }

    printf("===========================\n");
}

bool validate_index(const Index *idx)
{
    if (idx == NULL) {
        return false;
    }

    if (idx->bucket_count == 0) {
        return false;
    }

    if (idx->buckets == NULL) {
        return false;
    }

    for (size_t i = 0; i < idx->bucket_count; i++) {

        IndexEntry *cur = idx->buckets[i];

        while (cur != NULL) {

            /* Every entry must have a word */
            if (cur->word == NULL || *cur->word == '\0') {
                return false;
            }

            /* File array must exist if files are stored */
            if (cur->file_count > 0 && cur->files == NULL) {
                return false;
            }

            /* Count cannot exceed allocated capacity */
            if (cur->file_count > cur->file_capacity) {
                return false;
            }

            /* Capacity should not be zero if an entry exists */
            if (cur->file_capacity == 0) {
                return false;
            }

            /* Every stored file must be valid */
            for (size_t j = 0; j < cur->file_count; j++) {

                if (cur->files[j].filename == NULL ||
                    *cur->files[j].filename == '\0') {
                    return false;
                }

                if (cur->files[j].wordcount == 0) {
                    return false;
                }
            }

            /* Entry must actually belong to this bucket */
            size_t expected_bucket =
                hash_word(cur->word) % idx->bucket_count;

            if (expected_bucket != i) {
                return false;
            }

            cur = cur->next;
        }
    }

    return true;
}