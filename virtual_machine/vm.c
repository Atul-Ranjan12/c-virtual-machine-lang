#include "vm.h"
#include "../debug/debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Global vm instance
VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() { resetStack(); }

void freeVM() {}

// Run function actually handles the interpretation
static InterpreterResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = pop();                                                          \
    double a = pop();                                                          \
    push(a op b);                                                              \
  } while (false)

#ifdef DEBUG_TRACE_EXECUTION
  printf("          ");
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
    printf("[ ");
    printValue(*slot);
    printf(" ]");
  }
  printf("\n");
  dissassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

  for (;;) {
    uint8_t instruction = READ_BYTE();
    switch (instruction) {
    case OP_RETURN: {
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    case OP_NEGATE:
      push(-pop());
      break;
    case OP_ADD:
      BINARY_OP(+);
      break;
    case OP_SUBSTRACT:
      BINARY_OP(-);
      break;
    case OP_MULTIPLY:
      BINARY_OP(*);
      break;
    case OP_DIVIDE:
      BINARY_OP(/);
      break;
    }
  }

#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

// Sets the vm up and then proceeds with the interpretation
InterpreterResult interpret(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = chunk->code;

  resetStack();

  return run();
}

// Push operation for the stack
void push(Value value) {
  if (vm.stackTop - vm.stack >= STACK_MAX) {
    fprintf(stderr, "Stack overflow\n");
    exit(1);
  }
  *vm.stackTop = value;
  vm.stackTop++;
}

// Pop operation for the stack
Value pop() {
  if (vm.stackTop == vm.stack) {
    fprintf(stderr, "Stack underflow\n");
    exit(1);
  }
  vm.stackTop--;
  return *vm.stackTop;
}
