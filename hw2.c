#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// TODO: 
// -accept variable size matrices
// -check proper dimensions for dot product
// -test

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
int getRowLength(char* filename, FILE* fp);
int getColLength(char* filename, FILE* fp);
int matrixFileIsValid(char* filename, FILE* fp);
int rowsSameLength(char* filename, FILE* fp);
int validCharsOnly(char* filename, FILE* fp);
int validNumbersOnly(char* filename, FILE* fp);

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
    printf("Printing mat1 * mat2\n\n");
    printMatrix(mat3ptr);
    printf("Exiting from main\n\n");
    pthread_exit(NULL);
    return 0;
}

// all rows must be same length
// every char must be a space, newline, digit, or minus sign
// numbers must be either digits only or a dash followed by digits
int matrixFileIsValid(char* filename, FILE* fp) {
    char c;
    fp = fopen(filename, "r");
    c = fgetc(fp);

    if (validCharsOnly(filename, fp) == -1) {
        return -1;
    } 
    if (rowsSameLength(filename, fp) == -1) {
        return -1;
    }
    if (validNumbersOnly(filename, fp) == -1) {
        return -1;
    }
    return 1;
}

int rowsSameLength(char* filename, FILE* fp) {
    char c;
    int localRowLength = 0, globalRowLength = 0;
    fp = fopen(filename, "r");
    c = fgetc(fp);
    // First we get the length of the first line
    while (c != '\n') {
        while (c == ' ') {
            c = fgetc(fp);
        }
        localRowLength++;
        while (c != ' ' &&  c!='\n') { // Skip until the next space or end of line
            c = fgetc(fp);
        }
    }
    // If the first line is empty then we raise an error
    if (localRowLength == 0) {
        printf("Error: Row length can't be zero!");
        return -1;
    }
    globalRowLength = localRowLength;
    c = fgetc(fp);
    // Now we compare the length of each line to the length of the first line
    while (c != EOF) {
        localRowLength = 0;
        while (c != '\n' && c!=EOF) {
            while (c == ' ') {
                c = fgetc(fp);
            }
            localRowLength++;
            while (c!=' ' &&  c!='\n' && c!=EOF) { // Skip until the next space or end of line
                c = fgetc(fp);
            }
        }
        c = fgetc(fp);
        if (localRowLength != globalRowLength) {
            printf("Error: Rows must be same size!");
            return -1;
        }
    }
    return 1;
}

// Takes a filename and an empty file pointer
// Makes sure there are no characters other than '-', ' ', '\n', digits, and EOF 
int validCharsOnly(char* filename, FILE* fp) {
    char c;
    int colLength = 0;
    fp = fopen(filename, "r");
    c = fgetc(fp);
    while (c != EOF) {
        if (!isdigit(c) && c!='\n' && c!=EOF && c!='-' && c!=' ') {
            printf("ERROR: Dashes, digits, spaces, and newlines only!");
            return -1;
        }
        c = fgetc(fp);
    }
    return 1;
}

// Takes a filename and an empty file pointer
// Makes sure '-' only appears before a string of digits
int validNumbersOnly(char* filename, FILE* fp) {
    char c;
    fp = fopen(filename, "r");
    c = fgetc(fp);
    while (c != EOF) {
        if (c == '-') { // Detected the start of a possible negative number
            c = fgetc(fp); // make sure the character after a negative sign is a digit
            if (!isdigit(c)) {
                printf("ERROR: minus sign followed by non digit");
                return -1;
            } else { // make sure the characters following a dash are digits only
                c = fgetc(fp);
                while (c != ' ' && c != EOF && c != '\n') {
                    if (!isdigit(c)) {
                        printf("ERROR: digit followed by invalid char");
                        return -1;
                    }
                    c = fgetc(fp);
                }
            }
        }
        if (isdigit(c)) { // make sure number without a dash is digits only
            c = fgetc(fp);
            while (c != ' ' && c != '\n' && c != EOF) {
                if (!isdigit(c)) {
                    printf("ERROR: digit followed by invalid char");
                    return -1;
                }
                c = fgetc(fp);
            }
        }
        c = fgetc(fp);
    }
    return 1;
} 

// Takes the filename and an empty file pointer.
// Returns the length of the first row of a matrix file.
int getRowLength(char* filename, FILE* fp) {
    char c;
    int rowLength = 0;
    fp = fopen(filename, "r");
    c = fgetc(fp);
    while (c != '\n') { // Stop iterating after one line
        // Skip past all the spaces
        while (c == ' ') {
            c = fgetc(fp);
        }
        rowLength++;
        while (c != ' ' &&  c!='\n') { // Skip until the next space or end of line
            c = fgetc(fp);
        }
    }
    return rowLength;
}

// Takes the filename and an empty file pointer.
// Returns the column length of a matrix file.
// Simply counts the number of newline characters.
int getColLength(char* filename, FILE* fp) {
    char c;
    int colLength = 0;
    fp = fopen(filename, "r");
    c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            colLength += 1;
        }
        c = fgetc(fp);
    }
    colLength += 1;
    return colLength;
}

// Takes a filename (for a text file), an empty file pointer, and an empty matrix
// and returns a filled matrix.  
// Assumes that matrix entries are delimited by spaces (any number) and that
// matrix rows are delimited by newlines.
//
// PROBLEMS
// assumes single digit entries
// assumes consistent row lengths
// doesn't handle negative numbers
int* fileToMatrix(char* filename, FILE* fp, int* matrix) {

    // If the number of columns in a row is inconsistant we want to throw an error
    // If any of the characters are not digits or space then we want to thrown an error
    // We need to know the number of rows and columns...we'll do that
    // Then have an array of points where each pointer points to an array
    // of numbers.

    // First we'll check the format
    // Should have nothing other than spaces, digits, new lines, and subtraction
    // Then we'll get the row length, then the column length
    
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

// UPDATES NEEDED FOR DYNAMIC CODE
// Takes a matrix index and returns the next index.
//
void updateRowCol(int* row, int *col) {
    if (*col >= 12) {
        *col = 0;
        *row = *row + 1;
    } else {
        *col = *col+1;
    }
}

// UPDATES NEEDED FOR DYNAMIC CODE
// This function is very important.  It takes the row and column numbers
// of a matrix (0 indexed) and returns the corresponding flat array index.
// We use this method for matrix indexing because in our implementation 
// we choose to pass matrices to functions as type int* (rather than int**).
int getIndex(int row, int col) {
    return row*12+col;
}

// UPDATES NEEDED FOR DYNAMIC CODE
// Takes two input matrices 'mat1', 'mat2', and output matrix 'mat3',
// a row 'mat1row' and a column 'mat2col'.  It takes the dot product of 
// the indicated row and column and stores it in the proper location of mat3  
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

// UPDATES NEEDED FOR DYNAMIC CODE
// A threaded function to calculate one row of the output of the dot product
// of two matrices.  The row number is specified in 'params'.  So are the 
// two input matrices and the output matrix.  Essentially we iterate through
// the columns of 'mat2' and do the dot product with the indicated row of 'mat1'.
void* calcOutputRow(void *params) {
    struct thread_params *p = (struct thread_params*)params;
    printf("Creating thread: %d\n", p->row);
    for (int col=0; col<12; col++) {
        rowTimesColumn(p->mat1, p->mat2, p->mat3, p->row, col);
    }
    printf("Exiting thread: %d\n\n", p->row);
    pthread_exit(NULL);
}

// UPDATES NEEDED FOR DYNAMIC CODE
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