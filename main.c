#include "commons/common.h"
#include <stdio.h>

#include "chunk/chunk.h"
#include "debug/debug.h"
#include "virtual_machine/vm.h"


// The main function
int main(int argc, const char* argv[]) {
    initVM();

    Chunk chunk;

    initChunk(&chunk);
    int constant = addConstant(&chunk, 1);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);
    // Write the index
    constant = addConstant(&chunk, 3);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_ADD, 123);

    constant = addConstant(&chunk, 4);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_DIVIDE, 123);
    writeChunk(&chunk, OP_NEGATE, 123);
    writeChunk(&chunk, OP_RETURN, 124);

    // dissassembleChunk(&chunk, "test chunk");

    InterpreterResult res = interpret(&chunk);

    // Free the memory from virtual machine
    freeVM();
    // Free chunk
    freeChunk(&chunk);
    return 0;
}
