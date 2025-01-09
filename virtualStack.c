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

    for (int i = 0; i < file_length; i++){
        // Loading one byte of the file into the buffer
        fread(&buffer, 1, 1, file_ptr);
        // Converting the buffer into binary representation
        file_byte_one = convertion_from_binary_to_decimal((int)buffer);
        // Storing the binary representation
        memcpy(&file[index_file_position], file_byte_one, 8);
        index_file_position += 8;
        free(file_byte_one);
    }

    // Ensuring the buffer used to store file data is null terminated
    file[file_length * BITS_IN_BYTES] = '\0';



    // Dynamic Allocation memory for the virtual stack structure
    struct VirtualStack *virtual_stack = (struct VirtualStack*) malloc(sizeof(struct VirtualStack));
    // Allocate a stack for the virtual stack structure
    virtual_stack->stack = (BYTE *) calloc(MAX_NUM_FUNCTIONS, sizeof(BYTE));
    virtual_stack->number_of_functions_total = 0;

    // Initialise memory register array to 0
    for (int i = 0; i < 8; i++) {virtual_stack->memory_register[i] = 0;}
    // Allocates memory for an array of pointers to function structures
    virtual_stack->functions_array = (struct Function**) malloc(MAX_NUM_FUNCTIONS *
        sizeof(struct Function*));

    // Initalise each function in the function array
    for (int i = 0; i < MAX_NUM_FUNCTIONS; i++) {
        // Allocate memory for the fucntion structure in the array
        virtual_stack->functions_array[i] = (struct Function*) malloc(sizeof(struct Function));
        // Allocates memory of pointers to Instruction strucutres within each function
        virtual_stack->functions_array[i]->instructions = (struct Instruction**) malloc(MAX_NUM_FUNCTIONS
            * sizeof(struct Instruction));

        for (int j = 0; j < MAX_NUM_INSTRUCTIONS; j++){
            virtual_stack->functions_array[i]->instructions[j] = (struct Instruction*) malloc(sizeof(struct Instruction));
            }
    }

    // Reading into the file to load instructions into virtual stack
    int current_function_index = DEFAULT_VALUE;

    while (END_PADDING < index_file_position){
        parse_function(file, &index_file_position, virtual_stack->functions_array[current_function_index],
        (virtual_stack->functions_array[current_function_index]->instructions));
        virtual_stack->number_of_functions_total++;
        current_function_index++;
    }


    virtual_stack->stack[0] = DEFAULT_VALUE;
    virtual_stack->stack[2] = DEFAULT_VALUE;
    virtual_stack->frame_pointer = DEFAULT_VALUE;
    virtual_stack->program_counter = DEFAULT_VALUE;
    virtual_stack->return_value = DEFAULT_VALUE;
    int main_function_location = get_main_function(virtual_stack);
    virtual_stack->current_function_position = main_function_location;
    virtual_stack->stack_function_position = DEFAULT_VALUE;


    for (int i = 0; i < 128; i++){
        if (i == 0){
            virtual_stack->function_stack[i] = DEFAULT_VALUE;
            continue;
        }
        virtual_stack->function_stack[i] - NO_VALUE;
    }

    (virtual_stack->stack_pointer) = (virtual_stack->functions_array[main_function_location]->
        num_arguments_function) + 2;
    virtual_stack->stack[1] = (virtual_stack->stack_pointer);



    while(TRUE) {
        buffer_overflow(virtual_stack);
        int current_function_position = virtual_stack->current_function_position;
        int current_instruction_position = virtual_stack->program_counter;

        struct Instruction *current_instruction = virtual_stack->functions_array[current_function_position]
            ->instructions[current_instruction_position];

        int *current_address_one = &current_instruction->address_one
        int *current_address_type_one = &current_instruction->address_type_one
        int *current_address_two = &current_instruction->address_two
        int *current_address_type_two = &current_instruction->address_type_two  
        int operation_code = current_instruction->opcode;


        switch(operation_code){
            case 0:
                // 000 MOVE
                MOVE(virtual_stack, current_address_one, current_address_type_one, current_address_two, current_address_type_two);
                break;
            case 1:
                // 001 CALL
                CALL(virtual_stack, current_address_one, current_address_type_one, current_address_two, current_address_type_two);
                break;
            case 2:
                // 010 POP
                POP(virtual_stack, current_address_one, current_address_type_one);
                break;
            case 3:
                // 011 RETURN
                RETURN(virtual_stack);
                break;
            case 4:
                // 100 ADD
                ADD(virtual_stack, current_address_one, current_address_two);
                break;
            case 5:
                // 101 AND
                AND(virtual_stack, current_address_one, current_address_two);
                break;
            case 6:
                // 110 NOT
                NOT(virtual_stack, current_address_one);
                break;
            case 7:
                // 111 EQUAL
                EQUAL(virtual_stack, current_address_one);
                break;
        }
    }

    free_all_memory(virtual_stack, file);
    return 0;

}

char *convertion_from_decimal_to_binary(int decimal){
    int position_index = DEFAULT_VALUE;
    char *binary = (char*) malloc(BITS_IN_BYTES * sizeof(char) + 1);
    unsigned bit_mask_position = 1 << 7;
    while(bit_mask_position > DEFAULT_VALUE){
        if ((decimal & bit_mask_position) == 0){binary[position_index] = '0';}
        else {binary[position_index] = '1';}
        position_index++;
        bit_mask_position = bit_mask_position >> 1;
    }
    binary[8] = '\0';
    return binary;
}

char *convertion_from_binary_to_decimal(int decimal){
    int decimal = DEFAULT_VALUE, intermediary_value = DEFAULT_VALUE;
    int offset = 1;

    for (int i = number_bits - 1; i>=0; i--){
        intermediary_value = (int) (binary[i] - '0');
        decimal = decimal + (intermediary_value * offset);
        offset = 2 * offset;
    }
    return decimal;
}

void free_all(struct VirtualStack *virtual_stack, char *file){
    for (int i=0; i < MAX_NUM_FUNCTIONS; i++){
        for (int j=0; j<MAX_NUM_INSTRUCTIONS; j++){free(virtual_stack->functions_array[i]->instructions[j])};
    free(virtual_stack->functions_array[i]->instructions);
    free(virtual_stack->functions_array[i]);
    }
    free(virtual_stack->functions_array);
    free(virtual_stack->stack);
    free(virtual_stack);
    free(file);
}

void parse_function(char *file, int *index_position, struct Function *function, struct Instruction **instruction){
    // For Function Struct.
    int num_instructions = DEFAULT_VALUE;
    int current_function_id = NO_FUNCTION_ID;
    int num_arguments = DEFAULT_VALUE;

    // For Instruction Struct.
    int address_one = DEFAULT_VALUE;
    int address_type_two = DEFAULT_VALUE;
    int address_two = DEFAULT_VALUE;
    int address_type_one = DEFAULT_VALUE;
    int instruction_opcode = DEFAULT_VALUE;


    *index_position -= BITS_IN_BYTES;
    num_instructions = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_BYTES);
    function->num_instructions = num_instructions;

    for (int i = num_instructions - 1; i >= 0; i--){
        *index_position -= 3;
        instruction_opcode = convertion_from_binary_to_decimal(&file[*index_position], 3);
        instruction[i]->opcode = instruction_opcode;

        switch (instruction_opcode){
            case 3:
                initalise_instruction(NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, &instruction[i]);
                break;
            case 0:
            case 1:
            case 4:
            case 5:
            // Move, Call, Add, And cases
                *index_position -= 2;
                address_type_one = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);
                address_one = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));
                *index_position -= 2;
                address_type_two = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_two);
                address_two = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_two));
                initialise_instruction(address_one, address_two, address_type_one, address_type_two, &instruction[i]);
                break;
            case 2:
            case 6:
            case 7:
			// Pop, Not, Equal cases.
                *index_position -= 2;
                address_type_one = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);
                address_one = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));
                initialise_instruction(address_one, -1, address_type_one, -1, &instruction[i]);
                break;
        }
    }

    *index_position -= BITS_IN_HALF_BYTE;
    num_arguments = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);
    *index_position -= BITS_IN_HALF_BYTE;
    current_function_id = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);
    function->function_id = current_function_id;
    function->num_arguments_function = num_arguments;
    function->instructions = instruction;
}


// Compute the size of the address
int get_size_instruct_address(int address_type){
    switch(address_type){
        case 0: return 8;
        case 1: return 3;
        case 2: return 7;
        case 3: return 7;
    }
    return DEFAULT_VALUE;
}


// Need to initalise the instruction struct
void initialise_instruction(int add_one, int add_two, int address_type_one, int address_type_two,
                                struct Instruction **instruction){
    (*instruction)->address_one = add_one;
    (*instruction)->address_type_one = address_type_one;
    (*instruction)->address_two = add_two;
    (*instruction)->address_type_two = address_type_two;
}

// The index of where the main function exists
int main_function_location(struct VirtualStack *virtual_stack){
    for (int i = 0; i < virtual_stack->number_of_functions_total; i++){
        if (virtual_stack_>functions_array[i]->function_id == 0){
            return 1;
        }
    }
    return NO_VALUE;
}


// The index of where the function exists
int get function_location(int locate, struct VirtualStack *virtual_stack){
    if (virtual_stack->functions_array[i]->function_id == 0){
        return 1;
    }
    return NO_VALUE;
}



/* EXECUTING INSTRUCTIONS*/




