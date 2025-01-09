#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "virtualStack.h"
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

    printf("1\n");

//     for (int i = 0; i < file_length; i++) {
//     printf("Byte %d: %02x\n", i, (unsigned char)file[i]);
// }


    // Opening the file (rb -> binary read mode)
    FILE *file_ptr = fopen(filename, "rb");
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

    if (!file) {
        printf("Error: Memory allocation failed for 'file'.\n");
        exit(1);
    }

    // Point to individual bytes during processing
    char *file_one_byte;

    printf("Reading binary file content...\n");
    for (int i = 0; i < file_length; i++) {
        size_t read_size = fread(&buffer, 1, 1, file_ptr);
        if (read_size != 1) {
            printf("Error: Failed to read byte %d from file.\n", i);
            exit(1);
        }

        char *file_one_byte = convertion_from_decimal_to_binary((int)buffer);
        if (!file_one_byte) {
            printf("Error: Conversion to binary failed for byte %d (Value: %d).\n", i, buffer);
            exit(1);
        }

        printf("Read byte %d: Decimal=%d, Binary=%s\n", i, buffer, file_one_byte);

        memcpy(&file[index_file_position], file_one_byte, 8);
        index_file_position += 8;

        free(file_one_byte);
    }

    printf("Validating binary content...\n");
    for (int i = 0; i < index_file_position; i++) {
        if (file[i] != '0' && file[i] != '1') {
            printf("Error: Invalid character '%c' (Hex: %02x) at position %d.\n", 
                    file[i], (unsigned char)file[i], i);
            exit(1);
        }
    }
    printf("Binary content validated successfully.\n");



    printf("2\n");
    // Ensuring the buffer used to store file data is null terminated
    file[file_length * BITS_IN_BYTES] = '\0';

    // Debugging: Verify the binary file content
    printf("Binary File Content (Raw Data):\n");
    for (int i = 0; i < index_file_position; i++) {
        // Print the binary data character by character
        printf("Index %d: '%c' (Hex: %02x)\n", i, file[i], (unsigned char)file[i]);
        if (file[i] != '0' && file[i] != '1') {
            printf("Error: Invalid binary character '%c' at index %d. Expected '0' or '1'.\n", file[i], i);
            exit(1);
        }
    }
    printf("\nFinished verifying binary file content.\n");




    // Dynamic Allocation memory for the virtual stack structure
    struct VirtualStack *virtual_stack = (struct VirtualStack*) malloc(sizeof(struct VirtualStack));
    // Allocate a stack for the virtual stack structure
    virtual_stack->stack = (BYTE *) calloc(MAX_NUM_FUNCTIONS, sizeof(BYTE));
    virtual_stack->num_functions_total = 0;

    // Initialise memory register array to 0
    for (int i = 0; i < 8; i++) {virtual_stack->memory_register[i] = 0;}
    // Allocates memory for an array of pointers to function structures
    virtual_stack->functions_array = (struct Function**) malloc(MAX_NUM_FUNCTIONS *
        sizeof(struct Function*));


    printf("3\n");
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

    // Loop to parse functions from the file until all padding bits are processed
    while (END_PADDING < index_file_position) {
        // Parse a single function from the file and store it in the function array
        parse_function(file, &index_file_position, virtual_stack->functions_array[current_function_index],
                    (virtual_stack->functions_array[current_function_index]->instructions));
        
        // Increment the count of total functions in the virtual stack
        virtual_stack->num_functions_total++;
        
        // Move to the next function index
        current_function_index++;
    }

    printf("4\n");

    // Initialize important virtual stack properties to default values
    virtual_stack->stack[0] = DEFAULT_VALUE;       // Initialize the base stack value
    virtual_stack->stack[2] = DEFAULT_VALUE;       // Initialize program counter position
    virtual_stack->frame_pointer = DEFAULT_VALUE;  // Initialize frame pointer
    virtual_stack->program_counter = DEFAULT_VALUE;// Initialize program counter
    virtual_stack->return_value = DEFAULT_VALUE;   // Initialize return value placeholder

    // Find the main function's starting location and set it as the current function position
    int main_function_location = get_main_function_location(virtual_stack);
    virtual_stack->current_function_position = main_function_location;

    if (main_function_location == NO_VALUE) {
    printf("Error: Main function not found.\n");
    exit(1);
}

if (virtual_stack->functions_array[main_function_location] == NULL) {
    printf("Error: Function at location %d is NULL.\n", main_function_location);
    exit(1);
}

if (virtual_stack->functions_array[main_function_location]->num_arguments_function < 0) {
    printf("Error: Invalid number of arguments for main function.\n");
    exit(1);
}

    // Initialize stack function position to default
    virtual_stack->stack_function_position = DEFAULT_VALUE;

    // Initialize the function stack
    for (int i = 0; i < 128; i++) {
        if (i == 0) {
            // Set the first function stack value to default
            virtual_stack->function_stack[i] = DEFAULT_VALUE;
            continue;
        }
        // Set other function stack values to indicate no value
        virtual_stack->function_stack[i] = NO_VALUE;
    }
    printf("5\n");

    // Set the stack pointer to account for the number of arguments in the main function
    (virtual_stack->stack_pointer) = (virtual_stack->functions_array[main_function_location]->num_arguments_function) + 2;
    printf("5a\n");

    // Store the updated stack pointer in the stack
    virtual_stack->stack[1] = (virtual_stack->stack_pointer);
    printf("5b\n");

    // Begin executing instructions in a continuous loop
    while (TRUE) {
        // Check for stack overflows
        buffer_overflow(virtual_stack);
        printf("5c\n");

        // Get the current function and instruction positions
        int current_function_position = virtual_stack->current_function_position;
        int current_instruction_position = virtual_stack->program_counter;
        printf("5d\n");

        // Fetch the current instruction to be executed
        struct Instruction *current_instruction = virtual_stack->functions_array[current_function_position]
            ->instructions[current_instruction_position];
            printf("5e\n");

        // Get the addresses and types associated with the current instruction
        int *current_address_one = &current_instruction->address_one;
        int *current_address_type_one = &current_instruction->address_type_one;
        int *current_address_two = &current_instruction->address_two;
        int *current_address_type_two = &current_instruction->address_type_two;
        printf("5f\n");

        // Get the operation code of the current instruction
        int operation_code = current_instruction->opcode;

        printf("6\n");

        // Perform the operation based on the opcode using a switch-case structure
        switch (operation_code) {
            case 0:
                // 000 MOVE - Move values between memory/registers
                MOVE(virtual_stack, current_address_one, *current_address_type_one, current_address_two, *current_address_type_two);
                break;
            case 1:
                // 001 CALL - Call a function
                CALL(virtual_stack, current_address_one, *current_address_type_one, current_address_two, *current_address_type_two);
                break;
            case 2:
                // 010 POP - Pop a value from the stack
                POP(virtual_stack, current_address_one, *current_address_type_one);
                break;
            case 3:
                // 011 RETURN - Return from a function
                RETURN(virtual_stack);
                break;
            case 4:
                // 100 ADD - Add values from two addresses and store the result
                ADD(virtual_stack, current_address_one, current_address_two);
                break;
            case 5:
                // 101 AND - Perform bitwise AND between two values
                AND(virtual_stack, current_address_one, current_address_two);
                break;
            case 6:
                // 110 NOT - Perform bitwise NOT on a value
                NOT(virtual_stack, current_address_one);
                break;
            case 7:
                // 111 EQUAL - Check if a value is equal to zero
                EQUAL(virtual_stack, current_address_one);
                break;
        }
    }

    // Free all allocated memory and clean up before exiting
    free_all(virtual_stack, file);
    return 0; 
}

// Function to convert a decimal number to an 8-bit binary string
char *convertion_from_decimal_to_binary(int decimal) {
    // Initialize the position index for the binary array
    int position_index = DEFAULT_VALUE; 
    // Allocate memory for an 8-bit binary string (+1 for null terminator)
    char *binary = (char*) malloc(BITS_IN_BYTES * sizeof(char) + 1); 
    unsigned bit_mask_position = 1 << 7; // Set the bitmask to the highest bit (128 for an 8-bit number)

    // Loop through all 8 bits
    while (bit_mask_position > DEFAULT_VALUE) {
        if ((decimal & bit_mask_position) == 0) { 
            // If the current bit is 0, set the corresponding binary array position to '0'
            binary[position_index] = '0';
        } else {
            // If the current bit is 1, set the corresponding binary array position to '1'
            binary[position_index] = '1';
        }
        position_index++; // Move to the next position in the binary array
        bit_mask_position = bit_mask_position >> 1; // Shift the bitmask one position to the right
    }

    binary[8] = '\0'; // Null-terminate the binary string
    return binary; // Return the binary string
}

// Function to convert a binary string to a decimal number
int convertion_from_binary_to_decimal(char *binary, int number_bits) {
    int decimal = 0;
    int offset = 1;

    for (int i = number_bits - 1; i >= 0; i--) {
        if (binary[i] == '1') {
            decimal += offset;
        } else if (binary[i] == '0') {
            // Valid binary character
        } else {
            printf("Error: Invalid binary character '%c' at position %d. Expected '0' or '1'.\n",
                   binary[i], i);
            exit(1);
        }
        offset *= 2;
    }
    return decimal;
}

// Function to free all dynamically allocated memory
void free_all(struct VirtualStack *virtual_stack, char *file) {
    // Free each instruction in each function
    for (int i = 0; i < MAX_NUM_FUNCTIONS; i++) {
        for (int j = 0; j < MAX_NUM_INSTRUCTIONS; j++) {
            free(virtual_stack->functions_array[i]->instructions[j]); // Free individual instructions
        }
        free(virtual_stack->functions_array[i]->instructions); // Free the instruction array of the current function
        free(virtual_stack->functions_array[i]); // Free the current function structure
    }

    free(virtual_stack->functions_array); // Free the array of function pointers
    free(virtual_stack->stack); // Free the stack array
    free(virtual_stack); // Free the virtual stack structure itself
    free(file); // Free the file buffer
}


// Function to parse a single function from the binary file and populate the function and its instructions
void parse_function(char *file, int *index_position, struct Function *function, struct Instruction **instruction) {
    // Variables for Function struct
    int num_instructions = DEFAULT_VALUE;  // Number of instructions in the function
    int current_function_id = NO_FUNCTION_ID;  // Function ID
    int num_arguments = DEFAULT_VALUE;  // Number of arguments in the function

    // Variables for Instruction struct
    int address_one = DEFAULT_VALUE;  // First address in the instruction
    int address_type_one = DEFAULT_VALUE;  // Type of the first address
    int address_two = DEFAULT_VALUE;  // Second address in the instruction
    int address_type_two = DEFAULT_VALUE;  // Type of the second address
    int instruction_opcode = DEFAULT_VALUE;  // Operation code for the instruction

    // Move back by 1 byte to read the number of instructions
    *index_position -= BITS_IN_BYTES;
    num_instructions = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_BYTES);
    function->num_instructions = num_instructions;  // Store the number of instructions in the function struct

    // Loop through each instruction in the function
    for (int i = num_instructions - 1; i >= 0; i--) {
        *index_position -= 3;  // Move back 3 bits to read the instruction opcode
        instruction_opcode = convertion_from_binary_to_decimal(&file[*index_position], 3);
        instruction[i]->opcode = instruction_opcode;  // Store the opcode in the instruction struct

        switch (instruction_opcode) {
            case 3:
                // Case for RETURN instruction
                initialise_instruction(NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, &instruction[i]);
                break;
            case 0:
            case 1:
            case 4:
            case 5:
                // Cases for MOVE, CALL, ADD, AND instructions
                *index_position -= 2;  // Move back 2 bits to read the first address type
                address_type_one = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);  // Move back based on the size of the first address
                address_one = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));

                *index_position -= 2;  // Move back 2 bits to read the second address type
                address_type_two = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_two);  // Move back based on the size of the second address
                address_two = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_two));

                // Initialize the instruction with the parsed addresses and types
                initialise_instruction(address_one, address_two, address_type_one, address_type_two, &instruction[i]);
                break;
            case 2:
            case 6:
            case 7:
                // Cases for POP, NOT, EQUAL instructions
                *index_position -= 2;  // Move back 2 bits to read the first address type
                address_type_one = convertion_from_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);  // Move back based on the size of the first address
                address_one = convertion_from_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));

                // Initialize the instruction with only the first address
                initialise_instruction(address_one, -1, address_type_one, -1, &instruction[i]);
                break;
        }
    }

    // Move back to parse the number of arguments and function ID
    *index_position -= BITS_IN_HALF_BYTE;
    num_arguments = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);
    *index_position -= BITS_IN_HALF_BYTE;
    current_function_id = convertion_from_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);

    // Store the parsed function ID, number of arguments, and instructions in the function struct
    function->function_id = current_function_id;
    function->num_arguments_function = num_arguments;
    function->instructions = instruction;

    printf("Raw Binary for Num Instructions: %.*s\n", BITS_IN_BYTES, &file[*index_position]);
    printf("Raw Binary for Function ID: %.*s\n", BITS_IN_HALF_BYTE, &file[*index_position - BITS_IN_HALF_BYTE]);


    printf("Parsed Function: ID=%d, Num Instructions=%d, Num Arguments=%d\n",
       function->function_id, function->num_instructions, function->num_arguments_function);

}

// Compute the size of an address based on its type
int get_size_instruct_address(int address_type) {
    switch (address_type) {
        case 0: return 8;  // Value: 8 bits
        case 1: return 3;  // Register address: 3 bits
        case 2: return 7;  // Stack address: 7 bits
        case 3: return 7;  // Pointer address: 7 bits
    }
    return DEFAULT_VALUE;  // Default size if invalid address type
}

// Initialize the Instruction struct with provided values
void initialise_instruction(int add_one, int add_two, int address_type_one, int address_type_two, struct Instruction **instruction) {
    (*instruction)->address_one = add_one;
    (*instruction)->address_type_one = address_type_one;
    (*instruction)->address_two = add_two;
    (*instruction)->address_type_two = address_type_two;
}

// Get the location of the main function (function ID 0) in the function array
int get_main_function_location(struct VirtualStack *virtual_stack) {
    for (int i = 0; i < virtual_stack->num_functions_total; i++) {
        if (virtual_stack->functions_array[i]->function_id == 0) {
            return i;  // Return the index of the main function
        }
    }
    return NO_VALUE;  // Return NO_VALUE if the main function is not found
}

// Get the location of a specific function by its ID
int get_function_location(int locate, struct VirtualStack *virtual_stack) {
    for (int i = 0; i < virtual_stack->num_functions_total; i++) {
        if (virtual_stack->functions_array[i]->function_id == locate) {
            return i;  // Return the index of the function with the specified ID
        }
    }
    return NO_VALUE;  // Return NO_VALUE if the function is not found
}

void empty_registers(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: empty_registers
    * Description: Clear out registers since stack frame popped.
    *************************************************************/

		for(int i = 0; i<8; i++)
    {
        virtual_stack->memory_register[i] = 0;
    }
    return;
}


/* EXECUTING INSTRUCTIONS*/

// MOVE - moving the value from one memory location to another
void MOVE(struct VirtualStack *virtual_stack, int *address_one, int address_type_one,
            int *address_two, int address_type_two){
    int frame_pointer = virtual_stack->frame_pointer;

    if (*address_one == 1 && address_type_one == 3 ){
        //Push to the top of the stack
        virtual_stack->stack_pointer += 1;
        virtual_stack->stack[frame_pointer + 1] += 1;
        buffer_overflow(virtual_stack);
        switch(address_type_two)
			{
				case 0:
					virtual_stack->stack[virtual_stack->stack_pointer] = *address_two;
					break;
				case 1:
				  virtual_stack->stack[virtual_stack->stack_pointer] = virtual_stack->memory_register[*address_two];
					break;
				case 2:
					virtual_stack->stack[virtual_stack->stack_pointer] = virtual_stack->stack[*address_two + frame_pointer];
					break;
			}
			increment_pointer_counter(virtual_stack);
			return;
    }

    if (address_type_two == POINTER_TYPE){
        *address_two = virtual_stack->stack[frame_pointer + *address_two];
        address_type_two = 2;
    }

    if (address_type_one == POINTER_TYPE){
        *address_one = virtual_stack->stack[frame_pointer + *address_one];
        address_type_one = 2;
    }

    if(address_type_one == 2 && *address_one == (frame_pointer + 2))
    {
        // PROGRAM COUNTER.
        switch(address_type_two)
        {
            case 0:
                // VALUE
                virtual_stack->program_counter = *address_two;
                break;
            case 1:
                // REGISTER ADDRESS.
                virtual_stack->program_counter = virtual_stack->memory_register[*address_two];
                break;
            case 2:
                // STACK ADDRESS.
                virtual_stack->program_counter = (BYTE) virtual_stack->stack[frame_pointer + *address_two];
                break;
        }
    }

    switch (address_type_two){
        case 0:
            switch (address_type_one){
                case 1:
                    virtual_stack->memory_register[*address_one] = *address_two;
                    break;
                case 2:
                    virtual_stack->stack[*address_one + frame_pointer] = (BYTE) *address_two;
                    break;
            }
            break;

        case 1:
            switch (address_type_one){
                case 1:
                    virtual_stack->memory_register[*address_one] = 
                        virtual_stack->memory_register[*address_two];
                    break;
                case 2:
                    virtual_stack->stack[*address_one + frame_pointer] = 
                        (BYTE) virtual_stack->memory_register[*address_two];
                    break;
            }
            break;

        case 2:
            switch (address_type_one){
                case 1:
                    virtual_stack->memory_register[*address_one] = 
                        virtual_stack->memory_register[*address_two + frame_pointer];
                    break;
                case 2:
                    virtual_stack->stack[*address_one + frame_pointer] = 
                        (BYTE) virtual_stack->memory_register[*address_two + frame_pointer];
            }
                    break;
    }
    increment_pointer_counter(virtual_stack);
}

// CALL
void CALL(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two){
    buffer_overflow(virtual_stack);
    if (address_type_one == 3){
        *address_one = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];
    }

    virtual_stack->previous_frame_pointer = virtual_stack->frame_pointer;
    int previous_frame_pointer = virtual_stack->previous_frame_pointer;
    virtual_stack->frame_pointer = virtual_stack->stack_pointer+1;
    virtual_stack->stack[virtual_stack->frame_pointer] = virtual_stack->stack[previous_frame_pointer];

    virtual_stack->stack[(virtual_stack->frame_pointer) + 2] = 0;
    virtual_stack->stack[previous_frame_pointer + 2]++;
    virtual_stack->program_counter = 0;

    int function_location = get_function_location(*address_two, virtual_stack);
    int number_of_args = virtual_stack->functions_array[function_location]->num_arguments_function;

    for(int i=0; i<number_of_args; i++){
        virtual_stack->stack[(virtual_stack->frame_pointer) + i + 3] = virtual_stack->stack[(virtual_stack->previous_frame_pointer) + i + *address_one];
    }

    virtual_stack->current_function_position = function_location;
    virtual_stack->stack_function_position++;
    virtual_stack->function_stack[virtual_stack->stack_function_position] = *address_two;

    virtual_stack->stack_pointer = virtual_stack->frame_pointer+number_of_args+2;
    virtual_stack->stack[virtual_stack->frame_pointer+1] = virtual_stack->stack_pointer;

    empty_registers(virtual_stack);
}

// Pop
void POP(struct VirtualStack *virtual_stack, int *address_one, int address_type_one){
    if (address_type_one == 3){
        *address_one = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];
    }
    int value = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];
    virtual_stack->return_value = value;
    increment_pointer_counter(virtual_stack);
}

// Return 
void RETURN(struct VirtualStack *virtual_stack){
    is_main(virtual_stack);
    int current_return_value = virtual_stack->return_value;
    int previous_frame = virtual_stack->previous_frame_pointer;
    virtual_stack->stack[virtual_stack->frame_pointer] = current_return_value;
    virtual_stack->stack_pointer = (virtual_stack->frame_pointer) - 1;

    int stack_index = (virtual_stack->frame_pointer)+1;
    int stack_end = (virtual_stack->stack_pointer)+1;
    for(int i = stack_index; i < stack_end; i++){ virtual_stack->stack[i] = 0;}

    virtual_stack->frame_pointer = previous_frame;
    virtual_stack->function_stack[virtual_stack->stack_function_position] = -1;
    virtual_stack->stack_function_position--;

    empty_registers(virtual_stack);

    int previous_function_position = get_function_location(virtual_stack->function_stack[virtual_stack->stack_function_position], virtual_stack);
    virtual_stack->current_function_position = previous_function_position;

    virtual_stack->program_counter = virtual_stack->stack[virtual_stack->frame_pointer + 2];

    increment_pointer_counter(virtual_stack);

}

// Add 
void ADD(struct VirtualStack *virtual_stack, int *address_one, int *address_two){
    int result = virtual_stack->memory_register[*address_one] + virtual_stack->memory_register[*address_two];
    virtual_stack->memory_register[*address_two] = result;
    increment_pointer_counter(virtual_stack);
}

// And
void AND(struct VirtualStack *virtual_stack, int *address_one, int *address_two){
    int result = virtual_stack->memory_register[*address_one] & virtual_stack->memory_register[*address_two];
    virtual_stack->memory_register[*address_two] = result;
    increment_pointer_counter(virtual_stack);
}

// Not
void NOT(struct VirtualStack *virtual_stack, int *register_address){
    virtual_stack->memory_register[*register_address] = ~(virtual_stack->memory_register[*register_address]);
    increment_pointer_counter(virtual_stack);
}

// Equal
void EQUAL(struct VirtualStack *virtual_stack, int *register_address){
    if (virtual_stack->memory_register[*register_address] == FALSE){virtual_stack->memory_register[*register_address] = TRUE;}
    else {virtual_stack->memory_register[*register_address] = FALSE;}
    increment_pointer_counter(virtual_stack);
}

/*HELPER FUNCTIONS*/
void buffer_overflow(struct VirtualStack *virtual_stack){
    if(((virtual_stack->stack_pointer) + (virtual_stack->frame_pointer)) >= 128) {
        printf("Stack Overflow!\n");
        exit(0);
    }

    if(virtual_stack->program_counter >= 128){
        printf("Stack Overflow!\n");
        exit(0);
    }
}

void increment_pointer_counter(struct VirtualStack *virtual_stack){
    int location = virtual_stack->frame_pointer;
    virtual_stack->program_counter++;
    virtual_stack->stack[location+2]++;
	buffer_overflow(virtual_stack);
    return;
}

int check_is_register(int address_type){
    if (address_type == 3){
        return TRUE;
    }
    return FALSE;
}

int is_register(int address){
    if (address == 1) return 1;
    return 0;
}

void is_main(struct VirtualStack *virtual_stack){
    if (virtual_stack->stack_function_position == 0){
        printf("%d\n", virtual_stack->return_value);
    }
    return;
}


