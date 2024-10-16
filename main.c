#include "commons/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk/chunk.h"
#include "debug/debug.h"
#include "virtual_machine/vm.h"

// // The main function
// int main(int argc, const char* argv[]) {
//     initVM();

//     Chunk chunk;

//     initChunk(&chunk);
//     int constant = addConstant(&chunk, 1);
//     writeChunk(&chunk, OP_CONSTANT, 123);
//     writeChunk(&chunk, constant, 123);
//     // Write the index
//     constant = addConstant(&chunk, 3);
//     writeChunk(&chunk, OP_CONSTANT, 123);
//     writeChunk(&chunk, constant, 123);

//     writeChunk(&chunk, OP_ADD, 123);

//     constant = addConstant(&chunk, 4);
//     writeChunk(&chunk, OP_CONSTANT, 123);
//     writeChunk(&chunk, constant, 123);

//     writeChunk(&chunk, OP_DIVIDE, 123);
//     writeChunk(&chunk, OP_NEGATE, 123);
//     writeChunk(&chunk, OP_RETURN, 124);

//     // dissassembleChunk(&chunk, "test chunk");

//     InterpreterResult res = interpret(&chunk);

//     // Free the memory from virtual machine
//     freeVM();
//     // Free chunk
//     freeChunk(&chunk);
//     return 0;
// }

// readFile reads a file in th epath
static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open the file at %s", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytesRead] = '\0';

  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  // Make functionality to run the file
  InterpreterResult res = interpret(source);
}

// Main function
int main(int argc, const char *argv[]) {
  initVM();
  const char *filePath = "./test.lang";
  runFile(filePath);

  return 0;
}
