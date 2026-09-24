#include "tokenizer.h"

#include <ctype.h>
#include <string.h>

char *clean_and_normalize(char *str) {
    if (!str || *str == '\0') {
        return str;
    }

    char *start = str;
    while (*start != '\0' && ispunct((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *str = '\0';
        return str;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && ispunct((unsigned char)*end)) {
        end--;
    }

    char *dest = str;
    while (start <= end) {
        *dest = tolower((unsigned char)*start);
        dest++;
        start++;
    }

    *dest = '\0';

    return str;
}
