#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "../memory/memory.h"

// Initialize a new chunk
void initChunk(Chunk *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

// Write to a chunk
void writeChunk(Chunk *chunk, uint8_t byte, int line) {
    // If chunk is out of capacity grow the array
    if (chunk->count + 1 > chunk->capacity) {
        int oldCapacity = chunk->capacity;
        // Increased the capacity
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

// Empty a chunk
void freeChunk(Chunk *chunk){
    // Free the code
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    // Free the lines
    FREE_ARRAY(int, chunk->lines, chunk->capacity);

    initChunk(chunk);
}

// Write to the constants array and return the index
int addConstant(Chunk *chunk, Value v){
    writeValueArray(&chunk->constants, v);
    return chunk->constants.count - 1;
}
