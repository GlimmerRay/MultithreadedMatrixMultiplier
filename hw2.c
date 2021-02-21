#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define NUM_THREADS 12

/*
Create some empty 12x12 matrices and an array of thread identifiers
*/
int mat1[12][12];
int mat2[12][12];
int mat3[12][12];
pthread_t threads[12];

/*
Create a list of structures to store parameters for threaded function
*/
struct thread_params {
    int *mat1;
    int *mat2;
    int *mat3;
    int row;
} t_params[12];

/*
Function Declarations
*/

void updateRowCol(int* row, int *col);
int* fileToMatrix(char* filename, FILE* fp, int* matrix);
int getIndex(int row, int col);
void rowTimesColumn(int *mat1, int *mat2, int* mat3, int mat1row, int mat2col);
void* calcOutputRow(void *params);
void printMatrix(int* matrix);
void printMatrix(int* matrix);

int main(int argc, char *argv[]){

    int* mat1ptr;
    int* mat2ptr;
    FILE *fp1, *fp2;

	if(argc!=3){
		fprintf(stderr,"Usage: ./filename <mat1> <mat2>\n");
		return -1;
	}
    
    mat1ptr = fileToMatrix(argv[1], fp1, (int*)mat1);
    mat2ptr = fileToMatrix(argv[2], fp2, (int*)mat2);
    int *mat3ptr = (int*)mat3;

    // Creates 12 threads, each of which calculates one row of the output matrix
    for (int i=0; i<12; i++) {
        struct thread_params t_params = {mat1ptr, mat2ptr, mat3ptr, i};
        pthread_create(&threads[i], NULL, calcOutputRow, (void *)&t_params);
        pthread_join(threads[i], NULL);
    }
    
    printf("Printing mat1\n\n");
    printMatrix(mat1ptr);
    printf("Printing mat2\n\n");
    printMatrix(mat2ptr);
    printf("Printing mat3\n\n");
    printMatrix(mat3ptr);
    printf("Exiting from main\n\n");
    pthread_exit(NULL);
    return 0;
}

// Takes a filename (for a text file), an empty file pointer, and an empty matrix
// and returns a filled matrix.  
// Assumes that matrix entries are delimited by spaces (any number) and that
// matrix rows are delimited by newlines.
int* fileToMatrix(char* filename, FILE* fp, int* matrix) {
    
    int c, number, index;
    int row = 0, col = 0;
    
    fp = fopen(filename, "r");
    while ((c = fgetc(fp)) != EOF) {
        if (c != ' ') {
            number = c - '0';
            index = getIndex(row, col);
            matrix[index] = number;
            updateRowCol(&row, &col);
        }
    }
    return matrix;
}

// Takes a matrix index and returns the next index.
// For example 0,1 becomes 0,2 and 1,11 becomes 2,0
void updateRowCol(int* row, int *col) {
    if (*col >= 12) {
        *col = 0;
        *row = *row + 1;
    } else {
        *col = *col+1;
    }
}

// This function is very important.  It takes the row and column numbers
// of a matrix (0 indexed) and returns the corresponding flat array index.
// I use this method for matrix indexing because in my implementation 
// I choose to pass matrices to functions as type int* (rather than int**).
int getIndex(int row, int col) {
    return row*12+col;
}

// Takes two input matrices 'mat1', 'mat2', an output matrix 'mat3',
// a row 'mat1row' and a column 'mat2col'.  It takes the dot product of 
// the column and row indicated by 'mat1row' and 'mat2col'
// and stores it in the proper index of mat3  
void rowTimesColumn(int *mat1, int *mat2, int* mat3, int mat1row, int mat2col) {
    int sum = 0, index1, index2, index3;
    for (int i=0; i<12; i++) {
        index1 = getIndex(mat1row, i);
        index2 = getIndex(i, mat2col);
        sum = sum + mat1[index1]*mat2[index2];
    }
    index3 = getIndex(mat1row, mat2col);
    mat3[index3] = sum;
}


// A threaded function to calculate one row of the output of the dot product
// of two matrices.  The row number is specified in 'params'.  So are the 
// two input matrices and the output matrix.  Essentially we iterate through
// the columns of 'mat2' and take the dot product with the indicated row of 'mat1'.
void* calcOutputRow(void *params) {
    struct thread_params *p = (struct thread_params*)params;
    printf("Creating thread: %d\n", p->row);
    for (int col=0; col<12; col++) {
        rowTimesColumn(p->mat1, p->mat2, p->mat3, p->row, col);
    }
    printf("Exiting thread: %d\n\n", p->row);
    pthread_exit(NULL);
}

// Pretty prints a matrix to the terminal
void printMatrix(int* matrix) {
    int index;
    for (int row=0; row<12; row++) {
        for (int col=0; col<12; col++) {
            index = getIndex(row, col);
            printf("%d ", matrix[index]);
        }
        printf("\n");
    }
    printf("\n");
}