import numpy as np

# This script is used for generating matrix files for testing the main application


# converts a matrix to a string in the same format that 
# the matrixMultiplier.c expects and saves it to tests/filename
def makeMatrixString(matrix, filename):
    matStr = ''
    for row in range(len(matrix)):
        for col in range(len(matrix[0])):
            matStr += str(matrix[row][col])
            if col < len(matrix[0]) - 1:
                matStr += ' '
            else:
                matStr += '\n'
    # remove the final newline character
    matStr = matStr[:-1]
    with open(filename, 'w') as f:
        f.write(matStr)


# Sample use
np.random.seed(seed=99)

x = np.random.randint(low=-100, high=100, size=(2,4))
y = np.random.randint(low=-100, high=100, size=(4,10))

makeMatrixString(x, 'tests/mat3.txt')
makeMatrixString(y, 'tests/mat4.txt')


