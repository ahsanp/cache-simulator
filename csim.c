#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "cachelab.h"

#define BUFF_SIZE 1024

int main(int argc, char **argv, char **envp)
{
    int hits, misses, evictions;
    int opt, set_bits_count, lines_count, byte_bits_count;
    char *trace_name;

    hits = misses = evictions = 0;
    set_bits_count = lines_count = byte_bits_count = 0;

    // get options
    if (argc < 9) {
        printf("Usage: %s "
               "-s [#sets] -E [#lines] -b [#byte bits] "
               "-t [#trace_file_name]", argv[0]);
    }
    while((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch(opt) {
        case 's':
            set_bits_count = atoi(optarg);
            break;
        case 'E':
            lines_count = atoi(optarg);
            break;
        case 'b':
            byte_bits_count = atoi(optarg);
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

    // initialize cache
    cache *instance_cache;
    init_cache(&instance_cache, set_bits_count, lines_count, byte_bits_count);
    // open file, start reading and updating counts per line
    FILE *f_stream = fopen(trace_name, "r");
    if (f_stream == NULL) {
        fprintf(stderr, "Could not open file (%s): %s", trace_name, strerror(errno));
    }

    char *buffer = malloc(BUFF_SIZE);
    int req_size; // size of request
    long address; // the memory address
    char req_type; // Kind of request (S, M, L)

    while (fgets(buffer, BUFF_SIZE, f_stream) != NULL) {
        sscanf(buffer, " %c %ld,%d", &req_type, &address, &req_size);
        update_counts(instance_cache, req_type, address,
                      &hits, &misses, &evictions);
    }
    fclose(f_stream);
    delete_cache(&instance_cache);

    printSummary(hits, misses, evictions);
    return 0;
}

void init_cache(cache **cache_pointer, int set_bits_count,
                int lines_count, int byte_bits_count)
{
    cache *new_cache = (cache *) malloc(sizeof(cache));
    if (new_cache == NULL) {
        fprintf(stderr, "Error allocating memory for cache: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int no_of_sets = 1 << set_bits_count;
    cache_set *cache_sets_array = (cache_set *) malloc(sizeof(cache_set) * no_of_sets);
    if (new_cache == NULL) {
        fprintf(stderr, "Error allocating memory for cache sets: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // prepare each cache set to hold lines_count number of lines
    for (int i = 0; i < no_of_sets; i++) {
        (cache_sets_array + i) -> tags = (long *) malloc(sizeof(long) * lines_count);
        (cache_sets_array + i) -> tags = (char *) malloc(sizeof(char) * lines_count);
    }
    new_cache -> sets = cache_sets_array;
    new_cache -> lines_count = lines_count;
    long byte_mask, set_mask;
    byte_mask = set_mask = 0;
    int byte_mask_length, set_mask_length;
    byte_mask_length = set_mask_length = 0;
    // make byte mask and assign in cache
    for (int i = 0; i < byte_bits_count; i++) {
        byte_mask = (byte_mask << 1) + 1;
        byte_mask_length++;
    }
    new_cache -> byte_mask = byte_mask;
    new_cache -> byte_mask_length = byte_mask_length;
    // make set mask and assign in cache
    for (int i = 0; i < set_bits_count; i++) {
        set_mask = (set_mask << 1) + 1;
        set_mask_length++;
    }
    new_cache -> set_mask = set_mask;
    new_cache -> set_mask_length = set_mask_length;

    *cache_pointer = new_cache;
}

void delete_cache(cache **cache_pointer)
{

}
void upate_counts(cache *instance_cache, long address,
                  int *hits, int *misses, int *evictions)
{

}
