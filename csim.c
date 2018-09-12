#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
#include "cachelab.h"

#define BUFF_SIZE 1024
#define MAX_HEX_DIGITS 17 // accomodate the termination character

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
    cache *instance_cache = init_cache(set_bits_count,
                                       lines_count, byte_bits_count);
    // open file, start reading and updating counts per line
    FILE *f_stream = fopen(trace_name, "r");
    if (f_stream == NULL) {
        fprintf(stderr, "Could not open file (%s): %s", trace_name, strerror(errno));
    }

    char *buffer = malloc(BUFF_SIZE);
    int req_size; // size of request
    unsigned long address;
    char req_type; // Kind of request (S, M, L)
    while (fgets(buffer, BUFF_SIZE, f_stream) != NULL) {
        sscanf(buffer, " %c %lx,%d", &req_type, &address, &req_size);
        if (req_type == 'I') {
            // Skipping instruction accesses
            continue;
        }
        update_counts(instance_cache, address, req_type,
                      &hits, &misses, &evictions);
    }
    fclose(f_stream);
    free(buffer);
    delete_cache(instance_cache);

    printSummary(hits, misses, evictions);
    return 0;
}

cache *init_cache(int set_bits_count, int lines_count,
                int byte_bits_count)
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
        unsigned long *valid_bits = (unsigned long *) malloc(sizeof(long) * lines_count);
        // it is important that valid_bits array must be initialized to 0
        // it will be used for the LRU policy as well
        // increment last used by one
        for (int j = 0; j < lines_count; j++) {
            *(valid_bits + j) = 0;
        }
        (cache_sets_array + i) -> valid_bits = valid_bits;
    }
    new_cache -> sets = cache_sets_array;
    new_cache -> lines_count = lines_count;
    new_cache -> no_of_sets = no_of_sets;
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

    return new_cache;
}

void delete_cache(cache *cache)
{
    int no_of_sets = cache -> no_of_sets;
    cache_set *sets = cache -> sets;
    for (int i = 0; i < no_of_sets; i++) {
        free((sets + i) -> tags);
        free((sets + i) -> valid_bits);
    }
    free(sets);
    free(cache);
}
void update_counts(cache *instance_cache, long address, char op,
                   int *hits, int *misses, int *evictions)
{
    // int needed_byte = address & (instance_cache -> byte_mask);
    int needed_set = ((address >> (instance_cache -> byte_mask_length)) &
                      (instance_cache -> set_mask)); // index into array of sets
    int needed_tag = ((address >> (instance_cache -> byte_mask_length)) >>
                      (instance_cache -> set_mask_length));
    // retrieve particular set from cache
    cache_set *target_set = (instance_cache -> sets) + needed_set;
    long *tags = target_set -> tags;
    unsigned long *valid_bits = target_set -> valid_bits;
    int lines_count = instance_cache -> lines_count;
    int line_index = -1;
    int least_used_index = -1;
    unsigned long max_valid_bit = 0;
    unsigned long min_valid_bit = ULONG_MAX;
    int i = 0;
    // see if there is a better way to do this
    while (i < lines_count) {
        // tag must be equal and valid_bit must not be 0
        if (*(valid_bits + i) && *(tags + i) == needed_tag) {
            // it will come here only once
            // and that too if the tag exists in the array
            line_index = i;
        }
        // see if current valid bit is smaller
        // if so replace the index and min_valid_bit
        if (*(valid_bits + i) < min_valid_bit) {
            least_used_index = i;
            min_valid_bit = *(valid_bits + i);
        }
        // see if the valid bit if bigger
        // this will be used to figure out the value
        // of the LRU value of the target cache line
        if (*(valid_bits + i) > max_valid_bit) {
            max_valid_bit = *(valid_bits + i);
        }
        i++;
    }
    // by this point we have either found the target line
    // or we will not have found the target line because it
    // doesn't exist in the cache. So we will allocate
    // the LRU based index for that particular block of memory
    if (line_index != -1) {
        // if found perform operation
        // this will always be a hit(s)
        *hits = *hits + 1;
        if (op == 'M') {
            *hits = *hits + 1;
        }

    } else {
        // if not found
        *misses = *misses + 1;
        if (*(valid_bits + least_used_index) != 0) {
            // this means that the cache was previously
            // allocated
            *evictions = *evictions + 1;
        }
        if (op == 'M') {
            *hits = *hits + 1;
        }
        *(tags + least_used_index) = needed_tag;
        line_index = least_used_index;
    }
    *(valid_bits + line_index) = max_valid_bit + 1; // increase LRU
}
