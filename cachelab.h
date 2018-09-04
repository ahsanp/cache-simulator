/* 
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#define MAX_TRANS_FUNCS 100

typedef struct trans_func{
  void (*func_ptr)(int M,int N,int[N][M],int[M][N]);
  char* description;
  char correct;
  unsigned int num_hits;
  unsigned int num_misses;
  unsigned int num_evictions;
} trans_func_t;

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */

/* Fill the matrix with data */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
void registerTransFunction(
    void (*trans)(int M,int N,int[N][M],int[M][N]), char* desc);

typedef struct {
    long *tags;
    unsigned long *valid_bits; // will be used for LRU policing
} cache_set;

typedef struct {
    cache_set *sets;
    int lines_count;
    int no_of_sets;
    long byte_mask;
    long set_mask;
    int byte_mask_length;
    int set_mask_length;
} cache;

void init_cache(cache **cache_pointer,
                int set_bits_count,
                int lines_count, int byte_bits_count);
void update_counts(cache *instance_cache, long address, char op,
                   int *hits, int *misses, int *evictions);
void delete_cache(cache **cache_pointer);

#endif /* CACHELAB_TOOLS_H */
