#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "cachelab.h"

typedef struct {
    long *tags;
    char *valid_bits;
} cache_set;

typedef struct {
    cache_sets *sets;
    long byte_mask;
    long set_mask;
    int byte_mask_length;
    int set_mask_length;
} cache;

int main(int argc, char **argv, char **envp)
{
    // usage: -s -E -b -t
    int opt, sets_count, lines_count;
    int byte_bits;
    char *trace_name;
    sets_count = lines_count = byte_bits = 0;
    // Get options
    if (argc < 9) {
        printf("Usage: %s "
               "-s [#sets] -E [#lines] -b [#byte bits] "
               "-t [#trace_file_name]", argv[0]);
    }
    while((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch(opt) {
        case 's':
            sets_count = atoi(optarg);
            break;
        case 'E':
            lines_count = atoi(optarg);
            break;
        case 'b':
            byte_bits = atoi(optarg);
            break;
        case 't':
            trace_name = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s "
                    "-s [#sets] -E [#lines] -b [#byte bits] "
                    "-t [#trace_file_name]", argv[0]);
            exit(EXIT_FAILURE);
        }
    }



    printSummary(0, 0, 0);
    return 0;
}
