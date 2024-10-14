#include "compiler.h"
#include "../chunk/chunk.h"
#include "../commons/common.h"
#include "../scanner/scanner.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG_PRINT_CODE
#include "../debug/debug.h"
#endif

// Parser struct defines a new parser
typedef struct Parser {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

// Precedence enum established teh precedence
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

// type for the parsing function
typedef void (*ParseFn)();

// parse rule structure
typedef struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// Set the global variables
Parser parser;
Chunk *compilingChunk;

// function declarations
static void binary();
static void unary();
static ParseRule *getRule(TokenType type);
static void expression();
static void parsePrecedence(Precedence precedence);

// errorAt function prints out the error on a token
static void errorAt(Token *token, const char *message) {
  if (parser.panicMode)
    return;

  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF)
    fprintf(stderr, " at end");
  else if (token->type == TOKEN_ERROR)
    ;
  else
    fprintf(stderr, " at '%.*s'", token->length, token->start);

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

// Reports an error at the token that we just consumed
static void error(const char *message) { errorAt(&parser.previous, message); }

// errorAtCurrent sends an error at the current token
// For casess where the parser hands out an error token
static void errorAtCurrent(const char *message) {

  errorAt(&parser.current, message);
}

// Advance function parses the next token
static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR)
      break;

    errorAtCurrent(parser.current.start);
  }
}

// consume function looks for a particular token type
static void consume(TokenType type, const char *message) {
  if (type == parser.current.type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

// Returns the current chunk that is being compiled
static Chunk *currentChunk() { return compilingChunk; }

// emitByte emits a bytecode instruction for the chunk
static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

// emitReturn returns the OP_RETURN byte
static void emitReturn() { emitByte(OP_RETURN); }

// endCompiler ends the compilation process
static void endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    dissassembleChunk(currentChunk(), "code");
  }
#endif
}

// Emit two bytes, where we have to emit one opcode
// followed by one byte operand
static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

// makeConstant makes a value a constant
static uint8_t makeConstant(Value value) {
  // addConstant returns the index of the constant
  int constantIdx = addConstant(currentChunk(), value);
  // Return if there are too many constants
  if (constantIdx > UINT8_MAX) {
    error("Too many constants in one chunk");
    return 0;
  }

  // Return the index
  return (uint8_t)constantIdx;
}

// emitConstant emits a constant bytecode
static void emitConstant(Value value) {
  emitBytes(OP_CONSTANT, makeConstant(value));
}

// number adds a number to the vm
static void number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

// parsePrecedence handles parsing of expressions with
// precedence
static void parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression");
    return;
  }

  prefixRule();

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    // printf("Calling infix rule for token: %.*s\n", parser.previous.length,
    //        parser.previous.start);
    infixRule();
  }
}

// The expression function finally puts it in a
// expression
static void expression() { parsePrecedence(PREC_ASSIGNMENT); }
// grouping parses a grouping expression
// asume ( has been consumed
static void grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression .");
}

// Parses unary expressions
static void unary() {
  TokenType operatorType = parser.previous.type;

  // // Compile the operand
  // expression();

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
  case TOKEN_BANG:
    emitByte(OP_NOT);
    break;
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
    break;
  default:
    return;
  }
}

// binary parses binary expressions
static void binary() {
  TokenType operatorType = parser.previous.type;
  ParseRule *rule = getRule(operatorType);

  // Recursive call to parse it with precedence
  parsePrecedence((Precedence)rule->precedence + 1);

  switch (operatorType) {
  case TOKEN_BANG_EQUAL:
    emitBytes(OP_EQUAL, OP_NOT);
    break;
  case TOKEN_EQUAL_EQUAL:
    emitByte(OP_EQUAL);
    break;
  case TOKEN_GREATER:
    emitByte(OP_GREATOR);
    break;
  case TOKEN_GREATER_EQUAL:
    emitBytes(OP_LESS, OP_NOT);
    break;
  case TOKEN_LESS:
    emitByte(OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emitBytes(OP_GREATOR, OP_NOT);
    break;
  case TOKEN_PLUS:
    emitByte(OP_ADD);
    break;
  case TOKEN_STAR:
    emitByte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBSTRACT);
    break;
  default:
    return;
  }
}

// literal handles parsing of literal expressions
static void literal() {
  switch (parser.previous.type) {
  case TOKEN_TRUE:
    emitByte(OP_TRUE);
    break;
  case TOKEN_FALSE:
    emitByte(OP_FALSE);
    break;
  case TOKEN_NIL:
    emitByte(OP_NIL);
    break;
  default:
    return;
  }
}

// Rules for operator precedence parsing
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = {NULL, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

// function to get the rule
static ParseRule *getRule(TokenType type) { return &rules[type]; }

// The main compile funciton
bool compile(char *source, Chunk *chunk) {
  initScanner(source);
  compilingChunk = chunk;

  advance();
  expression();

  endCompiler();
  return !parser.hadError;
}
