#ifndef vm_debug_h
#define vm_debug_h

#include "../chunk/chunk.h"
#include <stdint.h>

// Function to disassemble a chunk
void dissassembleChunk(Chunk* chunk, char* name);
int dissassembleInstruction(Chunk* chunk, int offset);

#endif
