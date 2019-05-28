#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BISHOP 'B'
#define KING   'K'
#define KNIGHT 'N'
#define PAWN   'P'
#define QUEEN  'Q'
#define ROOK   'R'

int white_piece_off = 32;
const char* white_piece = "\x1B[97m";
int black_piece_off = 0;
const char* black_piece = "\x1B[94m";

const char* other = "\x1B[37m";
const char* reset = "\x1B[0m";

char** board = (void*)0;

void freeBoard()
{
  int i = 0;
  for(i = 0; i < 8; i++)
    free(board[i]);
  free(board);
}

void clearBoard()
{
  if(board) freeBoard();
  //allocate new memory
  board = (char**)malloc(sizeof(void*) * 8);
  memset(board, 0, sizeof(void*) * 8);
  int i = 0;
  for(i = 0; i < 8; i++)
  {
    char* row = (char*)malloc(sizeof(char) * 9);
    memset(row, 32, sizeof(char) * 8);
    row[8] = '\0';
    if(i == 0 || i == 7)
    {
      row[0] = ROOK   + ((i == 7) ? white_piece_off : black_piece_off);
      row[1] = KNIGHT + ((i == 7) ? white_piece_off : black_piece_off);
      row[2] = BISHOP + ((i == 7) ? white_piece_off : black_piece_off);
      row[3] = KING   + ((i == 7) ? white_piece_off : black_piece_off);
      row[4] = QUEEN  + ((i == 7) ? white_piece_off : black_piece_off);
      row[5] = BISHOP + ((i == 7) ? white_piece_off : black_piece_off);
      row[6] = KNIGHT + ((i == 7) ? white_piece_off : black_piece_off);
      row[7] = ROOK   + ((i == 7) ? white_piece_off : black_piece_off);
    }
    else if(i == 1 || i == 6)
    {
      int j = 0;
      for(j = 0; j < 8; j++)
        row[j] = PAWN  + ((i == 6) ? white_piece_off : black_piece_off);
    }
    board[i] = row;
  }
}

void printBoard()
{
  int i = 0;
  int j = 0;
  printf("%s  | A| B| C| D| E| F| G| H|  \n", other);
  for(i = 0; i < 8; i++)
  {
    printf("--+--+--+--+--+--+--+--+--+--\n");
    printf(" %i|", i + 1);
    for(j = 0; j < 8; j++)
    {
      int off = (board[i][j] > 96) ? white_piece_off : black_piece_off;
      printf(" %s%c%s|", (board[i][j] > 96) ? white_piece : black_piece,
                         (board[i][j] > 96) ? board[i][j] - off : board[i][j],
                         other);
    }
    printf("%i \n", i + 1);
  }
  printf("--+--+--+--+--+--+--+--+--+--\n");
  printf("  | A| B| C| D| E| F| G| H|  %s\n", reset);
}

void printScreen(const char* message)
{
  //clear the screen
  printf("\x1B[2J");
  //move cursor to top corner of screen
  printf("\x1B[1;1H");
  //print message in red
  printf("\x1B[91m");
  printf("%s\n%s", message, reset);
  //print board
  printBoard();
}

int main(int argc, char** argv)
{
  clearBoard();
  printScreen("Welcome");

  char

  while(getchar() != 27)
  {
    printScreen("This is a message lol");
  }
  return 0;
}
