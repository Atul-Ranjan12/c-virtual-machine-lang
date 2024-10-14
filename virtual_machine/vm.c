#include "vm.h"
#include "../debug/debug.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../compiler/compiler.h"

bool valueEquals(Value a, Value b);

// Global vm instance
VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() {
  resetStack();
  vm.stackTop = vm.stack;
}

void freeVM() {}

// runtimeError handles a runtime error in the script
static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

// peek operation peeks the value in the stack at a distance
static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

// isFalsey checks if a value is falsey or not
static bool isFalsey(Value value) {
  if (IS_NIL(value))
    return false;

  if (IS_BOOL(value) && !AS_BOOL(value))
    return true;

  return false;
}

// valueEquals checks if two values are equal
bool valueEquals(Value a, Value b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
  case VAL_NIL:
    return true;
  case VAL_NUMBER:
    return (AS_NUMBER(a) == AS_NUMBER(b));
  case VAL_BOOL:
    return (AS_BOOL(a) == AS_BOOL(b));
  default:
    return false;
  }
}

// Run function actually handles the interpretation
static InterpreterResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) && !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers for binary operations");          \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(pop());                                               \
    double a = AS_NUMBER(pop());                                               \
    push(valueType(a op b));                                                   \
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
      return INTERPRET_OK;
    }
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      // printf("Got constant: %.0f \n", (double)constant);
      push(constant);
      break;
    }
    case OP_NEGATE: {
      Value value = pop();
      if (!IS_NUMBER(value)) {
        runtimeError("Operand must be a number for negation");
        return INTERPRET_RUNTIME_ERROR;
      }

      push(NUMBER_VAL(-AS_NUMBER(value)));
      break;
    }
    case OP_ADD:
      BINARY_OP(NUMBER_VAL, +);
      break;
    case OP_SUBSTRACT:
      BINARY_OP(NUMBER_VAL, -);
      break;
    case OP_MULTIPLY:
      BINARY_OP(NUMBER_VAL, *);
      break;
    case OP_DIVIDE:
      BINARY_OP(NUMBER_VAL, /);
      break;
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(valueEquals(a, b)));
      break;
    }
    case OP_GREATOR:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
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
