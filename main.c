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
    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    // Write the index
    writeChunk(&chunk, constant, 123);
    writeChunk(&chunk, OP_RETURN, 124);

    dissassembleChunk(&chunk, "test chunk");

    // Free the memory from virtual machine
    freeVM();
    // Free chunk
    freeChunk(&chunk);
    return 0;
}
