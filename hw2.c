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

void updateRowCol(int* row, int *col) {
    if (*col >= 12) {
        *col = 0;
        *row = *row + 1;
    } else {
        *col = *col+1;
    }
}

int getIndex(int row, int col) {
    return row*12+col;
}

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

void* calcOutputRow(void *params) {
    struct thread_params *p = (struct thread_params*)params;
    printf("Creating thread: %d\n", p->row);
    for (int col=0; col<12; col++) {
        rowTimesColumn(p->mat1, p->mat2, p->mat3, p->row, col);
    }
    printf("Exiting thread: %d\n\n", p->row);
    pthread_exit(NULL);
}

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