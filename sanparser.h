#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum piece{
  UNKNOWN,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING
}Piece;

typedef struct san{
  Piece piece;
  char src[2];
  char cap;
  char dest[2];
  int suffix;
}San;

San* parse(char* input);
