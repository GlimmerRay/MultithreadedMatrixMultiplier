#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// Note: 
// Does not support floating point inputs
// Does not check for overflow
// This was for fun and learning not intended for production

struct thread_params {
    int* mat1;
    int* mat2;
    int* mat3;
    int row;
};

// to compile...
// gcc -o matrixMultiplier matrixMultiplier.c -lpthread

int numRows1, numCols1, numRows2, numCols2;

/*
Function Declarations
*/

void updateRowCol(int* row, int *col, int numCols);
int* fileToMatrix(FILE* fp, int* matrix);
int getIndex(int row, int col, int numCols);
void rowTimesColumn(int *mat1, int *mat2, int* mat3, int mat1row, int mat2col);
void* calcOutputRow(void *params);
void printMatrix(int* matrix, int numRows, int numCols);
void compatibleDimensions(FILE* fp1, FILE* fp2);
int getNumRows(FILE* fp);
int getNumCols(FILE* fp);
int matrixFileIsValid(FILE* fp);
int rowsSameLength(FILE* fp);
int validCharsOnly(FILE* fp);
int validNumbersOnly(FILE* fp);
int getNumber(FILE* fp, char c);

int main(int argc, char *argv[]){

    int* mat1, *mat2, *mat3;
    FILE *fp1, *fp2;
    

	if(argc!=3){
		fprintf(stderr,"Usage: ./filename <mat1> <mat2>\n");
		return -1;
	}

    fp1 = fopen(argv[1], "r");
    if (fp1 == NULL) {
        printf("Error: %s not found\n", argv[1]);
        exit(-1);
    }
    fp2 = fopen(argv[2], "r");
    if (fp2 == NULL) {
        printf("Error: %s not found\n", argv[2]);
        exit(-1);
    }

    // Error checks
    matrixFileIsValid(fp1);
    matrixFileIsValid(fp2);
    compatibleDimensions(fp1, fp2);

    // Important global variables
    numRows1 = getNumRows(fp1);
    numCols1 = getNumCols(fp1);
    numRows2 = getNumRows(fp2);
    numCols2 = getNumCols(fp2);

    printf("%d %d %d %d\n", numRows1, numCols1, numRows2, numCols2);

    // Allocate memory for each matrix
    mat1 = (int*) malloc(sizeof(int)*numRows1*numCols1);
    mat2 = (int*) malloc(sizeof(int)*numRows2*numCols2);
    mat3 = (int*) malloc(sizeof(int)*numRows1*numCols2);

    // Fill the input matrices with values
    mat1 = fileToMatrix(fp1, mat1);
    mat2 = fileToMatrix(fp2, mat2);

    // Do not need the file pointers anymore
    fclose(fp1);
    fclose(fp2);

    // Calculate the output matrix
    // Block until the calculations are complete
    for (int i=0; i<numCols2; i++) {
        pthread_t thread;
        struct thread_params t_params = {mat1, mat2, mat3, i};
        pthread_create(&thread, NULL, calcOutputRow, (void *)&t_params);
        pthread_join(thread, NULL);
    }
    
    printMatrix(mat3, numRows1, numCols2);
    pthread_exit(NULL);
    return 0;
}

// all rows must be same length
// every char must be a space, newline, digit, or minus sign
// numbers must be either digits only or a dash followed by digits
int matrixFileIsValid(FILE* fp) {
    char c;
    c = fgetc(fp);

    if (validCharsOnly(fp) == -1) {
        return -1;
    } 
    if (rowsSameLength(fp) == -1) {
        return -1;
    }
    if (validNumbersOnly(fp) == -1) {
        return -1;
    }
    return 1;
}

void compatibleDimensions(FILE* fp1, FILE* fp2) {
    if (getNumCols(fp1) != getNumRows(fp2)) {
        printf("Incompatible dimensions between mat1 and mat2 for matrix multiplication.\n");
        exit(-1);
    }
}

// Takes a filename (for a text file), an empty file pointer, and an empty matrix
// and returns a fills the matrix with data from the text file.
//
// Assumes that matrix entries are delimited by spaces and that
// matrix rows are delimited by newlines.
//
// Assumes the file represented by fp has been check by
// matrixFileIsValid()

int* fileToMatrix(FILE* fp, int* matrix) {
    
    int c, number, index;
    int row = 0, col = 0;
    int numCols = getNumCols(fp);

    while ((c = fgetc(fp)) != EOF) {
        if (c != ' ' && c != '\n') {
            number = getNumber(fp, c);
            index = getIndex(row, col, numCols);
            matrix[index] = number;
            updateRowCol(&row, &col, numCols);
        }
    }
    return matrix;
}

// assumes the file represented by fp has been checked by
// matrixFileIsValid()
int getNumber(FILE* fp, char c) {
    int sign;
    int num = 0;
    if (c == '-') {
        sign = -1;
        c = fgetc(fp);
    } else {
        sign = 1;
    }

    while (c != ' ' && c != EOF && c != '\n') {
        num = num * 10;
        num += c - '0';
        c = fgetc(fp);
    }
    num = num * sign;
    return num;
}

int rowsSameLength(FILE* fp) {
    char c;
    int localRowLength = 0, globalRowLength = 0;
    c = fgetc(fp);
    // First we get the length of the first line

    while (c != '\n') {
        while (c == ' ') {
            c = fgetc(fp);
        }
        while (c != ' ' &&  c!='\n') { // Skip until the next space or end of line
            c = fgetc(fp);
        }
        while (c == ' ') {
            c = fgetc(fp);
        }
        localRowLength++;
    }
    // If the first line is empty then we raise an error
    if (localRowLength == 0) {
        printf("Error: Row length can't be zero!\n");
        exit(-1);
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
            printf("Error: Rows must be same size!\n");
            exit(-1);
        }
    }
    fseek(fp, 0, SEEK_SET);
    return 1;
}

// Takes a filename and an empty file pointer
// Makes sure there are no characters other than '-', ' ', '\n', digits, and EOF 
int validCharsOnly(FILE* fp) {
    char c;
    int colLength = 0;
    c = fgetc(fp);
    while (c != EOF) {
        if (!isdigit(c) && c!='\n' && c!=EOF && c!='-' && c!=' ') {
            printf("ERROR: Dashes, digits, spaces, and newlines only!\n");
            exit(-1);
        }
        c = fgetc(fp);
    }
    fseek(fp, 0, SEEK_SET);
    return 1;
}

// Takes a filename and an empty file pointer
// Makes sure '-' only appears before a string of digits
int validNumbersOnly(FILE* fp) {
    char c;
    c = fgetc(fp);
    while (c != EOF) {
        if (c == '-') { // Detected the start of a possible negative number
            c = fgetc(fp); // make sure the character after a negative sign is a digit
            if (!isdigit(c)) {
                printf("ERROR: minus sign followed by non digit\n");
                exit(-1);
            } else { // make sure the characters following a dash are digits only
                c = fgetc(fp);
                while (c != ' ' && c != EOF && c != '\n') {
                    if (!isdigit(c)) {
                        printf("ERROR: digit followed by invalid char\n");
                        exit(-1);
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
                    exit(-1);
                }
                c = fgetc(fp);
            }
        }
        c = fgetc(fp);
    }
    fseek(fp, 0, SEEK_SET);
    return 1;
} 

// Takes the filename and an empty file pointer.
// Returns the length of the first row of a matrix file.
int getNumCols(FILE* fp) {
    char c;
    int numCols = 0;
    c = fgetc(fp);
    while (c != '\n') { // Stop iterating after one line
        // Skip past all the spaces
        while (c == ' ') {
            c = fgetc(fp);
        }
        numCols++;
        while (c != ' ' &&  c!='\n') { // Skip until the next space or end of line
            c = fgetc(fp);
        }
    }
    fseek(fp, 0, SEEK_SET);
    return numCols;
}

// Takes the filename and an empty file pointer.
// Returns the number of rows of a matrix file.
// Simply counts the number of newline characters.
int getNumRows(FILE* fp) {
    char c;
    int colLength = 0;
    c = fgetc(fp);
    while (c != EOF) {
        if (c == '\n') {
            colLength += 1;
        }
        c = fgetc(fp);
    }
    colLength += 1;
    fseek(fp, 0, SEEK_SET);
    return colLength;
}

// Takes a matrix index and returns the next index.
void updateRowCol(int* row, int *col, int numCols) {
    if (*col >= numCols - 1) {
        *col = 0;
        *row = *row + 1;
    } else {
        *col = *col+1;
    }
}

// This function is very important.  It takes the row and column numbers
// of a matrix (0 indexed) and returns the corresponding flat array index.
// We use this method for matrix indexing because in our implementation 
// we choose to pass matrices to functions as type int* (rather than int**).
int getIndex(int row, int col, int numCols) {
    return row*numCols+col;
}

// Takes two input matrices 'mat1', 'mat2', and output matrix 'mat3',
// a row 'mat1row' and a column 'mat2col'.  It takes the dot product of 
// the indicated row and column and stores it in the proper location of 'mat3'.
// The location is determined by 'mat1row' and 'mat2col'.
void rowTimesColumn(int *mat1, int *mat2, int* mat3, int mat1row, int mat2col) {
    int sum = 0, index1, index2, index3;
    for (int i=0; i<numRows2; i++) {
        index1 = getIndex(mat1row, i, numCols1);
        index2 = getIndex(i, mat2col, numCols2);
        sum = sum + mat1[index1]*mat2[index2];
    }
    index3 = getIndex(mat1row, mat2col, numCols2);
    mat3[index3] = sum;
}

// A threaded function to calculate one row of the output of the dot product
// of two matrices.  The row number is specified in 'params'.  So are the 
// two input matrices and the output matrix.  Essentially we iterate through
// the columns of 'mat2' and do the dot product with the indicated row of 'mat1'.
void* calcOutputRow(void *params) {
    struct thread_params *p = (struct thread_params*)params;
    // printf("Creating thread: %d\n", p->row);
    for (int col=0; col<numCols2; col++) {
        rowTimesColumn(p->mat1, p->mat2, p->mat3, p->row, col);
    }
    // printf("Exiting thread: %d\n\n", p->row);
    pthread_exit(NULL);
}

// prints a matrix to the terminal
void printMatrix(int* matrix, int numRows, int numCols) {
    int index;
    for (int row=0; row<numRows; row++) {
        for (int col=0; col<numCols; col++) {
            index = getIndex(row, col, numCols);
            printf("%d ", matrix[index]);
        }
        printf("\n");
    }
    printf("\n");
}