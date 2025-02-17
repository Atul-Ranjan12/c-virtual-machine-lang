#include "scanner.h"
#include "../commons//common.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  const char *start;
  const char *current;
  int line;
} Scanner;

// Define a global variable for the scanner
Scanner scanner;

// Function to initialize the scaner
void initScanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 0;
}

// isAtEnd checks if the scanner has reached the end
bool isAtEnd() { return *scanner.current == '\0'; }

// makeToken returns a token after making it one
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

// errorToken makes an error token
static Token errorToken(const char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = scanner.start;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

// advance function increases the current pointer and returns
// the value associated with it
static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

// match function increases the current pointer but does
// not return the value, instead returns true or false
static bool match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;

  scanner.current++;
  return true;
}

// peek function returns the value of the current pointer
static char peek() { return *scanner.current; }

// peekNext function returns the value of the next pointer
static char peekNext() {
  if (isAtEnd())
    return '\0';
  return scanner.current[1];
}

// skipWhiteSpace skips all the whitespace to get to a
// token
static void skipWhiteSpace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n': {
      scanner.line++;
      advance();
      break;
    }
    case '/': {
      if (peekNext() == '/')
        while (peek() != '\n' && !isAtEnd())
          advance();
      else
        return;

      break;
    }
    default:
      return;
    }
  }
}

// isDigit checks if it is a number
static bool isDigit(char c) { return c >= '0' && c <= '9'; }

// number function lexes a number
static Token number() {
  while (isDigit(peek()))
    advance();

  if (peek() == '.' && isDigit(peekNext())) {
    advance();

    while (isDigit(peek()))
      advance();
  }

  return makeToken(TOKEN_NUMBER);
}

// string function lexes a string
static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }

  if (isAtEnd())
    return errorToken("Unterminated string");

  // Advance the closing quote
  advance();

  return makeToken(TOKEN_STRING);
}

// isAlpha function checks if it is an alphabet
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

// checkKeyword checks if the word is infact a
// keyword
static TokenType checkKeyword(int start, int length, const char *rest,
                              TokenType type) {
  if ((scanner.current - scanner.start == length + start) &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

// identifierType returns an TOKEN_IDENTIFIER
static TokenType identifierType() {
  switch (scanner.start[0]) {
  case 'a':
    return checkKeyword(1, 2, "nd", TOKEN_AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TOKEN_ELSE);
  case 'i':
    return checkKeyword(1, 1, "f", TOKEN_IF);
  case 'n':
    return checkKeyword(1, 2, "il", TOKEN_NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TOKEN_OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TOKEN_SUPER);
  case 'v':
    return checkKeyword(1, 2, "ar", TOKEN_VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  case 'f': {
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_FUN);
      }
    }
    break;
  }
  case 't':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  }

  return TOKEN_IDENTIFIER;
}

// identifier checks if it is an identifier
static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek()))
    advance();

  return makeToken(identifierType());
}

// scanToken scans the next token and returns it
Token scanToken() {
  skipWhiteSpace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();

  // Check if c is a digit
  if (isDigit(c))
    return number();

  // Check if it is an identifier (could also be a keyword)
  if (isAlpha(c))
    return identifier();

  switch (c) {
  case '(':
    return makeToken(TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_SEMICOLON);
  case ',':
    return makeToken(TOKEN_COMMA);
  case '.':
    return makeToken(TOKEN_DOT);
  case '-':
    return makeToken(TOKEN_MINUS);
  case '+':
    return makeToken(TOKEN_PLUS);
  case '/':
    return makeToken(TOKEN_SLASH);
  case '*':
    return makeToken(TOKEN_STAR);
  case '!':
    return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"':
    return string();
  }

  return errorToken("Unexpected character");
}
