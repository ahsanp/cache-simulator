/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

#define BLOCK_SIZE 8

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void transpose_32_32(int M, int N, int A[N][M], int B[M][N])
{
    int diag_cord, diag_val;
    int diag_flag = 0;
    for (int rr = 0; rr < N; rr += BLOCK_SIZE) {
        // move block row wise
        for (int cc = 0; cc < M; cc += BLOCK_SIZE) {
            // move block column wise
            for (int c = cc; c < cc + BLOCK_SIZE; c++) {
                // iterate through each column of block
                for (int r = rr; r < rr + BLOCK_SIZE; r++) {
                    // iterate through each row of block
                    if (r != c) {
                        B[r][c] = A[c][r];
                    } else {
                        diag_flag = 1;
                        diag_cord = r; // r == c
                        diag_val = A[r][c];
                    }
                }
                if (diag_flag) {
                    B[diag_cord][diag_cord] = diag_val;
                    diag_flag = 0;
                }
            }
        }
    }
}

void transpose_64_64(int M, int N, int A[N][M], int B[M][N])
{
    int a0, a1, a2, a3, a4, a5, a6, a7;
    // rr and cc are use to iterate over
    // rows and columns block wise respectively
    for (int rr = 0; rr < N; rr += BLOCK_SIZE) {
        for (int cc = 0; cc < M; cc += BLOCK_SIZE) {
            for (int iter = 0; iter < BLOCK_SIZE; iter++) {
                a0 = A[cc + iter][rr];
                a1 = A[cc + iter][rr + 1];
                a2 = A[cc + iter][rr + 2];
                a3 = A[cc + iter][rr + 3];
                if (iter == 0) {
                    a4 = A[cc + iter][rr + 4];
                    a5 = A[cc + iter][rr + 5];
                    a6 = A[cc + iter][rr + 6];
                    a7 = A[cc + iter][rr + 7];
                }
                B[rr][cc + iter] = a0;
                B[rr + 1][cc + iter] = a1;
                B[rr + 2][cc + iter] = a2;
                B[rr + 3][cc + iter] = a3;
            }
            for (int iter = BLOCK_SIZE - 1; iter > 0; iter--) {
                a0 = A[cc + iter][rr + 4];
                a1 = A[cc + iter][rr + 5];
                a2 = A[cc + iter][rr + 6];
                a3 = A[cc + iter][rr + 7];
                B[rr + 4][cc + iter] = a0;
                B[rr + 5][cc + iter] = a1;
                B[rr + 6][cc + iter] = a2;
                B[rr + 7][cc + iter] = a3;
            }
            B[rr + 4][cc] = a4;
            B[rr + 5][cc] = a5;
            B[rr + 6][cc] = a6;
            B[rr + 7][cc] = a7;
        }
    }
}

void transpose_61_67(int M, int N, int A[N][M], int B[M][N])
{
    for (int rr = 0; rr < N; rr += BLOCK_SIZE) {
        // move block row wise
        for (int cc = 0; cc < M; cc += BLOCK_SIZE) {
            // move block column wise
            for (int c = cc; c < cc + BLOCK_SIZE; c++) {
                // iterate through each column of block
                for (int r = rr; r < rr + BLOCK_SIZE; r++) {
                    // iterate through each row of block
                    if (r >= N || c >= M) {
                        // bounds checking
                        continue;
                    }
                    B[c][r] = A[r][c];
                }
            }
        }
    }
}

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == N && M == 32)
        transpose_32_32(M, N, A, B);
    else if (M == N && M == 64)
        transpose_64_64(M, N, A, B);
    else if (N == 61 && M == 67)
        transpose_61_67(M, N, A, B);
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
