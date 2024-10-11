#ifndef vm_vm_h
#define vm_vm_h
#include "../chunk/chunk.h"
#include <stdint.h>

typedef struct {
    Chunk* chunk;
    // Instruction pointer, points to the
    // instruction set and deferences it for
    // faster things
    uint8_t* ip;
} VM;

// Enum for the interpretation
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpreterResult;

void initVM();
void freeVM();
InterpreterResult interpret(Chunk* chunk);

#endif
