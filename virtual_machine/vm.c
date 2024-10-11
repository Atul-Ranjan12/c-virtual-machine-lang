#include "vm.h"
#include <stdint.h>
#include <stdlib.h>

// Global vm instance
VM vm;

void initVM() {}

void freeVM() {}

// Run function actually handles the interpretation
static InterpreterResult run() {
// Macro to dereference the ip
#define READ_BYTE() (*vm.ip++)

  for (;;) {
    uint8_t instruction = READ_BYTE();
    switch (instruction) {
    case OP_RETURN:
      return INTERPRET_OK;
    }
  }
#undef READ_BYTE
}

// Sets the vm up and then proceeds with the interpretation
InterpreterResult interpret(Chunk *chunk) {
    vm.chunk = chunk;
    vm.ip = chunk->code;

    return run();
}
