/*
 * cachelab.c - Cache Lab helper functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include "cachelab.h"
#include <time.h>

trans_func_t func_list[MAX_TRANS_FUNCS];
int func_counter = 0;

/*
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
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

    // Find matching tag, maximum LRU value and minimum LRU value line
    for (int i = 0; i < lines_count; i++) {
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

/*
 * initMatrix - Initialize the given matrix
 */
void initMatrix(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
            B[j][i]=rand();
        }
    }
}

void randMatrix(int M, int N, int A[N][M]) {
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
        }
    }
}

/*
 * correctTrans - baseline transpose function used to evaluate correctness
 */
void correctTrans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}



/*
 * registerTransFunction - Add the given trans function into your list
 *     of functions to be tested
 */
void registerTransFunction(void (*trans)(int M, int N, int[N][M], int[M][N]),
                           char* desc)
{
    func_list[func_counter].func_ptr = trans;
    func_list[func_counter].description = desc;
    func_list[func_counter].correct = 0;
    func_list[func_counter].num_hits = 0;
    func_list[func_counter].num_misses = 0;
    func_list[func_counter].num_evictions =0;
    func_counter++;
}
