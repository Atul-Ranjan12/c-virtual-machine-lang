#ifndef vm_chunk_h
#define vm_chunk_h

#include "../commons/common.h"
#include "../value/value.h"
#include <stdint.h>

// Operator instructions
typedef enum {
  OP_CONSTANT,
  OP_NEGATE,
  OP_RETURN,
  OP_PRINT,
  OP_JUMP_IF_FALSE,
  OP_JUMP,
  OP_LOOP,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_EQUAL,
  OP_GREATOR,
  OP_LESS,
  OP_ADD,
  OP_SUBSTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
} OpCode;

typedef struct Chunk {
  int capacity;         // Capacity of the array
  int count;            // Current count of the array
  uint8_t *code;        // Code
  ValueArray constants; // Constants
  int *lines;           // Store the lines of code
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);
// Write to the constants array and return the index
int addConstant(Chunk *chunk, Value v);

#endif
