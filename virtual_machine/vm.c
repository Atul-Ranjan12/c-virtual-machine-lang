#include "vm.h"
#include "../debug/debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../compiler/compiler.h"

// Global vm instance
VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() {
  resetStack();
  vm.stackTop = vm.stack;
}

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
      printf("%.0f", pop());
      // printf("Got here at OP_RETURN");
      // printf("\n");
      return INTERPRET_OK;
    }
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      // printf("Got constant: %.0f \n", (double)constant);
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
InterpreterResult interpret(char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpreterResult result = run();
  freeChunk(&chunk);
  return result;
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
  Value value = *vm.stackTop;
  return value;
}
