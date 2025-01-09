#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Common Definitions
#define BYTE unsigned char
#define TRUE 1
#define FALSE 0
#define DEFAULT_VALUE 0
#define NO_VALUE -1
#define NO_FUNCTION_ID -1
#define BITS_IN_HALF_BYTE 4
#define BITS_IN_BYTES 8
#define MAX_NUM_FUNCTIONS 128
#define MAX_NUM_INSTRUCTIONS 256
#define END_PADDING 7
#define POINTER_TYPE 3


struct Instruction {
    int address_one;
    int address_type_one;
    int address_two;
    int address_type_two;
    int opcode;
};

struct Function {
	int function_id;
	int num_arguments_function;
  struct Instruction **instructions;
  int num_instructions;
};

struct VirtualStack {
  BYTE *stack;
  int program_counter;
  int frame_pointer;
  int stack_pointer;
  int num_functions_total;
  struct Function **functions_array;
  int current_function_position;
  int previous_frame_pointer;
  int function_stack[128];
  int stack_function_position;
  int return_value;
  BYTE memory_register[8];
};

// READING IN FILES.
char *convertion_from_decimal_to_binary(int decimal);
int convertion_from_binary_to_decimal(char* binary, int num_bits);
void parse_function(char *file, int *index_position, struct Function *function, struct Instruction **instruction);
int get_size_instruct_address(int address_type);
void initialise_instruction(int address_one, int address_two, int address_type_one, int addres_type_two, struct Instruction **instruction);

// INITIALISING VIRTUAL STACK.
int get_function_location(int locate, struct VirtualStack *virtual_stack);
int get_main_function_location(struct VirtualStack *virtual_stack);
int get_function_location(int locate, struct VirtualStack *virtual_stack);


// END OF FILE ADMIN STUFF.
void free_all(struct VirtualStack *virtual_stack, char *file);

void MOVE(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two);
void CALL(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two);
void POP(struct VirtualStack *virtual_stack, int *address_one, int address_type_one);
void RETURN(struct VirtualStack *virtual_stack);
void ADD(struct VirtualStack *virtual_stack, int *address_one, int *address_two);
void AND(struct VirtualStack *virtual_stack, int *address_one, int *address_two);
void NOT(struct VirtualStack *virtual_stack, int *register_address);
void EQUAL(struct VirtualStack *virtual_stack, int *register_address);

void increment_pointer_counter(struct VirtualStack *virtual_stack);
void buffer_overflow(struct VirtualStack *virtual_stack);
int check_is_register(int address_type);
int is_register(int address);
void is_main(struct VirtualStack *virtual_stack);
void empty_registers(struct VirtualStack *virtual_stack);

#endif // VIRTUALSTACK_H
