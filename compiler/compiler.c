#include "compiler.h"
#include "../chunk/chunk.h"
#include "../commons/common.h"
#include "../object/object.h"
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
typedef void (*ParseFn)(bool canAssign);

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
static void binary(bool canAssign);
static void unary(bool canAssign);
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

// check function sees if a token type is of the required type
// without advancing the parser
static bool check(TokenType type) { return parser.current.type == type; }

// match function checks for the type and then advances
// the pointer returns true
static bool match(TokenType type) {
  if (!check(type))
    return false;

  advance();
  return true;
}

// emitConstant emits a constant bytecode
static void emitConstant(Value value) {
  emitBytes(OP_CONSTANT, makeConstant(value));
}

// number adds a number to the vm
static void number(bool canAssign) {
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

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    // printf("Calling infix rule for token: %.*s\n", parser.previous.length,
    //        parser.previous.start);
    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

// The expression function finally puts it in a
// expression
static void expression() { parsePrecedence(PREC_ASSIGNMENT); }
// grouping parses a grouping expression
// asume ( has been consumed
static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression .");
}

// Parses unary expressions
static void unary(bool canAssign) {
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
static void binary(bool canAssign) {
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
static void literal(bool canAssign) {
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

// string handles parsing of string expressions
static void string(bool canAssign) {
  emitConstant(OBJ_VAL(
      copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

// -------------------- Variables --------------------

// Functions for variables

// -------------------- Declaration

// To Declare variables, we add it to our list of constants
// in our vm

// identifierConstant adds the identifier to the chunks->constant
// array as an Obj and then returns the index
static uint8_t identifierConstant(Token *name) {
  // copyString handles allocation of string to objectString type
  // and returns the objectString which is then casted to OBJ_VAL t
  // to the Obj type, which is then made a constant
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

// parseVariable parses the variable and displays the error
// message :: returns the index of the variables position
static uint8_t parseVariable(const char *errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);
  return identifierConstant(&parser.previous);
}

// In the byte code variables are defines as one
// OP_DEFINE_GLOBAL command followed by the index
// of the actual variable
static void defineVariable(uint8_t variableIndex) {
  emitBytes(OP_DEFINE_GLOBAL, variableIndex);
}

// varDeclaration function defines the variable
static void varDeclaration() {
  // Var has already been consumed at this point
  uint8_t globalIndex = parseVariable("Expect variable name");

  if (match(TOKEN_EQUAL))
    expression();
  else
    emitByte(OP_NIL);

  consume(TOKEN_SEMICOLON, "Expect ; after variable declaration");

  // Define the variable
  defineVariable(globalIndex);
}

// -------------------- Reading Variables

static void namedVariable(Token name, bool canAssign) {
  uint8_t idx = identifierConstant(&name);

  if (canAssign && match(TOKEN_EQUAL)) {
    // Next token is an equal
    expression();
    emitBytes(OP_SET_GLOBAL, idx);
  }

  emitBytes(OP_GET_GLOBAL, idx);
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

// printStatement handles a print statement
static void printStatement() {
  // parse the expression
  expression();
  consume(TOKEN_SEMICOLON, "Expect ; after print statement");
  emitByte(OP_PRINT);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ; after expression");
  emitByte(OP_POP);
}

// statement parses different statements
static void statement() {
  if (match(TOKEN_PRINT))
    printStatement();
  else if (match(TOKEN_VAR))
    varDeclaration();
  else
    expressionStatement();

  // For now if not a print statement, it is an expression
  // expression();
  // consume(TOKEN_SEMICOLON, "Expect ; at the end of expression");
}

// For panic mode recovery
static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON)
      return;
    switch (parser.current.type) {
    case TOKEN_CLASS:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;

    default:; // Do nothing.
    }

    advance();
  }
}

// declaration parses decalrations
static void declaration() {
  statement();
  if (parser.panicMode)
    synchronize();
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
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
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
  while (!match(TOKEN_EOF)) {
    declaration();
  }

  endCompiler();
  return !parser.hadError;
}
