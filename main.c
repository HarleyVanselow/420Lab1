/* File:     
 *     pth_mat_vect.c 
 *
 * Purpose:  
 *     Computes a parallel matrix-vector product.  Matrix
 *     is distributed by block rows.  Vectors are distributed by 
 *     blocks.
 *
 * Input:
 *     m, n: order of matrix
 *     A, x: the matrix and the vector to be multiplied
 *
 * Output:
 *     y: the product vector
 *
 * Compile:  gcc -g -Wall -o pth_mat_mat main.c lab1_IO.c -lpthread
 * Usage:
 *     pth_mat_mat <thread_count>
 *
 * Notes:  
 *     1.  Local storage for A, x, y is dynamically allocated.
 *     2.  Number of threads (thread_count) should evenly divide both 
 *         m and n.  The program doesn't check for this.
 *     3.  We use a 1-dimensional array for A and compute subscripts
 *         using the formula A[i][j] = A[i*n + j]
 *     4.  Distribution of A, x, and y is logical:  all three are 
 *         globally shared.
 *
 * IPP:    Section 4.3 (pp. 159 and ff.).  Also Section 4.10 (pp. 191 and 
 *         ff.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lab1_IO.h"
#include "timer.h"


int n, thread_count;
int **A; int**B; int** C;

void Usage (char* prog_name);
void *Pth_mat_mat(void* rank);
void calculate_cell(int row, int col);

int main (int argc, char* argv[])
{
    double time, start, end;
    pthread_t* thread_handles;

    if (argc != 2) {
        Usage(argv[0]);
    }

    thread_count = atoi(argv[1]);
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    Lab1_loadinput(&A, &B, &n);
    C = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        C[i] = malloc(n * sizeof(int));
    }

    /*Calculating*/
    GET_TIME(start);
    for (int thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL,
            Pth_mat_mat, (void*) thread);
    }

    for (int thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }

    GET_TIME(end);
    time = end - start;

    Lab1_saveoutput(C, &n, time);

    for (int i = 0; i <=n; i++)
    {
        free(A[i]); free(B[i]); free(C[i]);
    }
    free(A); free(B); free(C);
    return 0;
}

/*--------------------------------------------------------------
* Function:  Usage
* Purpose:   print a message showing what the command line should
*            be, and terminate
* In arg :   prog_name
*/
void Usage (char* prog_name) {
    fprintf(stderr, "usage: %s <thread_count>\n", prog_name);
    exit(0);
}  /* Usage */


/*---------------------------------------------------------------
 * Function:       Pth_mat_vect
 * Purpose:        Multiply an mxn matrix by an nx1 column vector
 * In arg:         rank
 * Global in vars: A, x, m, n, thread_count
 * Global out var: y
 */
void *Pth_mat_mat(void* rank) {
    long my_rank = (long) rank;
    int i, j;
    int local_n = n/thread_count; 
    int init_row = 0;
    int final_row = n;
    int init_col = 0;
    int final_col = n;

    for (i = init_row; i < final_row; i++) {
        for (j = init_col; j < final_col; j++){
            calculate_cell(i, j);
        }
    }      

    return NULL;
}  /* Pth_mat_vect */

void calculate_cell(int row, int col){
    int cell = 0;
    for(int i = 0; i < n; i++){
        cell += A[row][i] * B[i][col]; 
    }
    C[row][col] = cell;
}
