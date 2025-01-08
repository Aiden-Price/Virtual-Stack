#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vstack.h"
#define BYTE unsigned char
#define BITS_IN_HALF_BYTE 4
#define BITS_IN_BYTES 8
#define MAX_NUM_FUNCTIONS 128
#define MAX_NUM_INSTRUCTIONS 256
#define END_PADDING 7
#define NO_FUNCTION_ID -1
#define DEFAULT_VALUE 0
#define NO_VALUE -1
#define TRUE 1
#define POINTER_TYPE 3

/* Workflow Procedure
    Parse the Binary File:
        Read and decode padding bits, function headers, opcodes, and function tails.
    Setup Memory and Registers:
        Initialize stack memory, stack pointer, frame pointer, and registers.
    Execute Opcodes:
        Implement logic for each opcode, such as moving values, arithmetic operations, and managing stack frames.
    Function Execution:
        Start with the main function, handle function calls (CAL), and return values (RET).
    Error Checking:
        Validate instructions, prevent illegal memory access, and handle infinite loops.*/

int main(int number_of_args, char **vargs) {
    // Retrieve file name from the second command line argument
    char* filename = vargs[1];

    if (number_of_args != 2) {
        printf("Invalid argument was passed");
        exit(0);
    }

    // Opening the file (rb -> binary read mode)
    FILE *file_ptr = fopen(filename, 'rb');
    // Moving a pointer to the end of the file
    fseek(file_ptr, 0, SEEK_END);
    /* Get the current positon of the file pointer 
    (corresponds to the file size) Which is in bytes */
    size_t file_length = ftell(file_ptr);
    // Resets the file pointer
    rewind(file_ptr);

    // Track current position in file
    int index_file_position = DEFAULT_VALUE;
    BYTE buffer;
    // Allocate memory for file content storage
    char *file = (char *) malloc(file_length * BITS_IN_BYTES + 1);
    // Point to individual bytes during processing
    char *file_one_byte;


}


