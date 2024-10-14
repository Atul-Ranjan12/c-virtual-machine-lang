#include "../chunk/chunk.h"
#include "../value//value.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int simpleInstruction(const char *name, int offset) {
  printf(" %s\n", name);

  return offset + 1;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t constantIdx = chunk->code[offset + 1];
  printf(" %-16s %4d '", name, constantIdx);
  printValue(chunk->constants.values[constantIdx]);
  printf("'\n");

  return offset + 2;
}

int dissassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    printf("   | ");
  else
    printf("%4d ", chunk->lines[offset]);

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);
  case OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset);
  case OP_NEGATE:
    return simpleInstruction("OP_NAGATE", offset);
  case OP_ADD:
    return simpleInstruction("OP_ADD", offset);
  case OP_SUBSTRACT:
    return simpleInstruction("OP_SUBSTRACT", offset);
  case OP_MULTIPLY:
    return simpleInstruction("OP_MULTIPLY", offset);
  case OP_DIVIDE:
    return simpleInstruction("OP_DIVIDE", offset);
  case OP_NIL:
    return simpleInstruction("OP_NIL", offset);
  case OP_TRUE:
    return simpleInstruction("OP_TRUE", offset);
  case OP_FALSE:
    return simpleInstruction("OP_FALSE", offset);
  case OP_NOT:
    return simpleInstruction("OP_NOT", offset);
  case OP_EQUAL:
    return simpleInstruction("OP_EQUAL", offset);
  case OP_GREATOR:
    return simpleInstruction("OP_GREATOR", offset);
  case OP_LESS:
    return simpleInstruction("OP_LESS", offset);
  default:
    return offset + 1;
  }
}

void dissassembleChunk(Chunk *chunk, char *name) {
  printf("== %s == \n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = dissassembleInstruction(chunk, offset);
  }
}
