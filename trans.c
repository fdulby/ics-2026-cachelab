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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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
    int a[8];

    if (M == 64 && N == 64) {
        for (int i = 0; i < N; i += 8) {
            for (int j = 0; j < M; j += 8) {
                /* The top-right quadrant waits in the top-left B rows. */
                for (int r = i; r < i + 4; ++r) {
                    a[0] = A[r][j]; a[1] = A[r][j + 1];
                    a[2] = A[r][j + 2]; a[3] = A[r][j + 3];
                    a[4] = A[r][j + 4]; a[5] = A[r][j + 5];
                    a[6] = A[r][j + 6]; a[7] = A[r][j + 7];
                    B[j][r] = a[0];     B[j + 1][r] = a[1];
                    B[j + 2][r] = a[2]; B[j + 3][r] = a[3];
                    B[j][r + 4] = a[4];     B[j + 1][r + 4] = a[5];
                    B[j + 2][r + 4] = a[6]; B[j + 3][r + 4] = a[7];
                }
                /* Exchange staged values with the transposed bottom-left. */
                for (int c = j; c < j + 4; ++c) {
                    a[0] = A[i + 4][c]; a[1] = A[i + 5][c];
                    a[2] = A[i + 6][c]; a[3] = A[i + 7][c];
                    a[4] = B[c][i + 4]; a[5] = B[c][i + 5];
                    a[6] = B[c][i + 6]; a[7] = B[c][i + 7];
                    B[c][i + 4] = a[0]; B[c][i + 5] = a[1];
                    B[c][i + 6] = a[2]; B[c][i + 7] = a[3];
                    B[c + 4][i] = a[4];     B[c + 4][i + 1] = a[5];
                    B[c + 4][i + 2] = a[6]; B[c + 4][i + 3] = a[7];
                    B[c + 4][i + 4] = A[i + 4][c + 4];
                    B[c + 4][i + 5] = A[i + 5][c + 4];
                    B[c + 4][i + 6] = A[i + 6][c + 4];
                    B[c + 4][i + 7] = A[i + 7][c + 4];
                }
            }
        }
    } else if (M == 32 && N == 32) {
        for (int i = 0; i < N; i += 8) {
            for (int j = 0; j < M; j += 8) {
                for (int r = i; r < i + 8; ++r) {
                    /* Finish reading the row before a B store can evict it. */
                    for (int c = 0; c < 8; ++c)
                        a[c] = A[r][j + c];
                    for (int c = 0; c < 8; ++c)
                        B[j + c][r] = a[c];
                }
            }
        }
    } else {
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < M; j += 16) {
                for (int r = i; r < N && r < i + 16; ++r) {
                    for (int c = j; c < M && c < j + 16; ++c)
                        B[c][r] = A[r][c];
                }
            }
        }
    }
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

