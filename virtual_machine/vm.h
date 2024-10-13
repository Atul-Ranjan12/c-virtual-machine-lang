#ifndef vm_vm_h
#define vm_vm_h
#include "../chunk/chunk.h"
#include "../value/value.h"
#include <stdint.h>

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    // Instruction pointer, points to the
    // instruction set and deferences it for
    // faster things
    uint8_t* ip;
    // Implement the stack in the vm to store
    // values
    Value stack[STACK_MAX];
    Value* stackTop;
} VM;

// Enum for the interpretation
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpreterResult;

void initVM();
void freeVM();
InterpreterResult interpret(char* source);

// Stack operations
void push(Value value);
Value pop();

#endif
