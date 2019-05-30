#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LEN 255

#include "sanparser.h"

typedef struct player{
  char* name;
  char* color;
  int off;
  int castle_lr;
  int castle_rr;
  int castle_k;
  int* pieces; //length 16 array with the locations of each piece. PPPPPPPPRRNNBBQK
  int* enpassant; //length 8 array containing enpassant info (0 means no enpassant 1 means enpassant)
  int* moves_len; //length 16 array wwith the lengths of each piece's move array (if -1 the moves array needs to be updated)
  int** moves; //length 16 array with the moves a piece can make
  int* captured; //length 6 array of the number of pieces PNBRQK
}Player;

//int white_piece_off = 32;
//const char* white_piece_color = "\x1B[97m";
//int black_piece_off = 0;
//const char* black_piece_color = "\x1B[90m";

const char* message_color = "\x1B[91m";
const char* other_color = "\x1B[36m";
const char* reset_color = "\x1B[0m";

char** board = (void*)0;
int num_of_turns = 0;

Player* white;
Player* black;

Player* buildPlayer(int white, char* name, char* color, int off)
{
  printf("%i\n", sizeof(struct player));
  Player* p = (Player*)malloc(sizeof(struct player));
  memset(p, 0, sizeof(struct player));

  int namelength = strlen(name);
  p->name = (char*)malloc(sizeof(char) * (namelength + 1));
  memset(p->name, 0, sizeof(char) * (namelength + 1));
  memcpy(p->name, name, sizeof(char) * namelength);

  int colorlength = strlen(color);
  p->color = (char*)malloc(sizeof(char) * (colorlength + 1));
  memset(p->color, 0, sizeof(char) * (colorlength + 1));
  memcpy(p->color, color, sizeof(char) * colorlength);

  p->off = off;

  p->pieces = (int*)malloc(sizeof(int) * 16);
  int i = 0;
  for(i = 0; i < 8; i++)
    p->pieces[i] = i + ((white == 0) ? 8 : 48);

  for(i = 0; i < 8; i++)
  {
    p->pieces[i + 8] = (i % 2 == 0) ? i : 8 - i;
    if(white != 0) p->pieces[i + 8] += 56;
  }

  p->enpassant = (int*)malloc(sizeof(int) * 8);
  memset(p->enpassant, 0, sizeof(int) * 16);

  p->moves_len = (int*)malloc(sizeof(int) * 16);
  for(i = 0; i < 16; i++) p->moves_len[i] = -1;

  p->moves = (int**)malloc(sizeof(int*) * 16);
  for(i = 0; i < 16; i++)
  {
    //28 is the maximum number of moves a single piece can take
    p->moves[i] = (int*)malloc(sizeof(int) * 30);
    memset(p->moves[i], 0, sizeof(int) * 30);
  }

  p->captured = (int*)malloc(sizeof(int) * 6);
  memset(p->captured, 0, sizeof(int) * 6);

}

void freePlayer(Player* player)
{
  free(player->name);
  free(player->color);
  free(player->pieces);
  free(player->moves_len);
  free(player->enpassant);

  free(player->moves);
  int i = 0;
  for(i = 0; i < 16; i++) free(player->moves[i]);

  free(player->captured);

  free(player);
}

void addLocation(Player* player, int pidx, int x, int y)
{
  player->moves[pidx][player->moves_len[pidx]] = y * 8 + x;
  player->moves_len[pidx] += 1;
}

int landableSpot(int sx, int sy, int dx, int dy)
{
  char piece = board[sy][sx];
  if(board[dy][dx] == 32) return 1;
  if(piece < 97)
    if(board[dy][dx] < 97)
      return 0;
    else
      return 2;
  else
    if(board[dy][dx] >= 97)
      return 0;
    else
      return 2;
}

void findMoves(Player* player)
{
  //(0, 0) is top left or A1 of the board
  int piece = 0;
  for(piece = 0; piece < 16; piece++)
  {
    if(player->moves_len[piece] != -1) continue; //don't recalculate moves if not needed
    player->moves_len[piece] = 0;

    int x = (player->pieces[piece]) % 8;
    int y = (player->pieces[piece]) / 8;

    if(piece < 8)
    {
      if(player->off == 0)
      {
        int y1 = y + 1;
        if(y1 < 8)
        {
          if(landableSpot(x, y, x, y1) == 1) addLocation(player, piece, x, y1);
          if(x < 7)
            if(landableSpot(x, y, x + 1, y1) == 2)
              addLocation(player, piece, x + 1, y1);
          if(x > 0)
            if(landableSpot(x, y, x - 1, y1) == 2)
              addLocation(player, piece, x - 1, y1);
        }
        if(y == 1)
          if(landableSpot(x, y, x, y1 + 1) == 1)
            addLocation(player, piece, x, y1 + 1);
      }
      else
      {
        int y1 = y - 1;
        if(y1 >= 0)
        {
          if(landableSpot(x, y, x, y1) == 1) addLocation(player, piece, x, y1);
          if(x < 7)
            if(landableSpot(x, y, x + 1, y1) == 2)
              addLocation(player, piece, x + 1, y1);
          if(x > 0)
            if(landableSpot(x, y, x - 1, y1) == 2)
              addLocation(player, piece, x - 1, y1);
        }
        if(y == 6)
          if(landableSpot(x, y, x, y1 - 1) == 1)
            addLocation(player, piece, x, y1 - 1);
      }
    }
    else if(piece < 10)
    {
      int i, j, k;
      int ec1 = 0;
      int ec2 = 0;
      int ec3 = 0;
      int ec4 = 0;
      for(i = 1; i < 8; i++)
      {
        int x1 = x + i;
        int x2 = x - i;
        int y1 = y + i;
        int y2 = y - i;

        if(x1 < 8 && ec1 == 0)
        {
          int spot = landableSpot(x, y, x1, y);
          if(spot == 0)
          {
            ec1++;
          }
          else
          {
            if(spot == 2) ec1++;
            addLocation(player, piece, x1, y);
          }
        }
        if(y1 >= 0 && ec2 == 0)
        {
          int spot = landableSpot(x, y, x, y1);
          if(spot == 0)
          {
            ec2++;
          }
          else
          {
            if(spot == 2) ec2++;
            addLocation(player, piece, x, y1);
          }
        }
        if(x2 >= 0 && ec3 == 0)
        {
          int spot = landableSpot(x, y, x2, y);
          if(spot == 0)
          {
            ec3++;
          }
          else
          {
            if(spot == 2) ec3++;
            addLocation(player, piece, x2, y);
          }
        }
        if(y2 >= 0 && ec4 == 0)
        {
          int spot = landableSpot(x, y, x, y2);
          if(spot == 0)
          {
            ec4++;
          }
          else
          {
            if(spot == 2) ec4++;
            addLocation(player, piece, x, y2);
          }
        }
      }
    }
    else if(piece < 12)
    {
      int x1 = x - 2;
      int x2 = x - 1;
      int x3 = x + 1;
      int x4 = x + 2;
      int y1 = y - 2;
      int y2 = y - 1;
      int y3 = y + 1;
      int y4 = y + 2;

      if(x1 >= 0) {
        if(y2 >= 0){
          if(landableSpot(x, y, x1, y2) != 0) addLocation(player, piece, x1, y2);
        }
        if(y3 < 8){
          if(landableSpot(x, y, x1, y3) != 0) addLocation(player, piece, x1, y2);
        }
      }
      if(x2 >= 0){
        if(y1 >= 0){
          if(landableSpot(x, y, x2, y1) != 0) addLocation(player, piece, x2, y1);
        }
        if(y4 < 8){
          if(landableSpot(x, y, x2, y4) != 0) addLocation(player, piece, x2, y4);
        }
      }
      if(x3 < 8){
        if(y1 >= 0){
          if(landableSpot(x, y, x3, y1) != 0) addLocation(player, piece, x3, y1);
        }
        if(y4 < 8){
          if(landableSpot(x, y, x3, y4) != 0) addLocation(player, piece, x3, y4);
        }
      }
      if(x4 < 8){
        if(y2 >= 0){
          if(landableSpot(x, y, x4, y2) != 0) addLocation(player, piece, x4, y2);
        }
        if(y3 < 8){
          if(landableSpot(x, y, x4, y3) != 0) addLocation(player, piece, x4, y3);
        }
      }
    }
    else if(piece < 14)
    {
      int i, j, k;
      int ec1 = 0;
      int ec2 = 0;
      int ec3 = 0;
      int ec4 = 0;
      for(i = 1; i < 8; i++)
      {
        int x1 = x + i;
        int x2 = x - i;
        int y1 = y + i;
        int y2 = y - i;

        if(x1 < 8 && y1 < 8 && ec1 == 0)
        {
          int spot = landableSpot(x, y, x1, y1);
          if(spot == 0)
          {
            ec1++;
          }
          else
          {
            if(spot == 2) ec1++;
            addLocation(player, piece, x1, y1);
          }
        }
        if(x1 < 8 && y2 >= 0 && ec2 == 0)
        {
          int spot = landableSpot(x, y, x1, y2);
          if(spot == 0)
          {
            ec2++;
          }
          else
          {
            if(spot == 2) ec2++;
            addLocation(player, piece, x1, y2);
          }
        }
        if(x2 >= 0 && y1 < 8 && ec3 == 0)
        {
          int spot = landableSpot(x, y, x2, y1);
          if(spot == 0)
          {
            ec3++;
          }
          else
          {
            if(spot == 2) ec3++;
            addLocation(player, piece, x2, y1);
          }
        }
        if(x2 >= 0 && y2 >= 0 && ec4 == 0)
        {
          int spot = landableSpot(x, y, x2, y2);
          if(spot == 0)
          {
            ec4++;
          }
          else
          {
            if(spot == 2) ec4++;
            addLocation(player, piece, x2, y2);
          }
        }
      }
    }
    else if(piece == 14)
    {
      int i, j, k;
      int ec1 = 0;
      int ec2 = 0;
      int ec3 = 0;
      int ec4 = 0;
      int ec5 = 0;
      int ec6 = 0;
      int ec7 = 0;
      int ec8 = 0;
      for(i = 1; i < 8; i++)
      {
        int x1 = x + i;
        int x2 = x - i;
        int y1 = y + i;
        int y2 = y - i;

        if(x1 < 8 && ec1 == 0)
        {
          int spot = landableSpot(x, y, x1, y);
          if(spot == 0)
          {
            ec1++;
          }
          else
          {
            if(spot == 2) ec1++;
            addLocation(player, piece, x1, y);
          }
        }
        if(y1 >= 0 && ec2 == 0)
        {
          int spot = landableSpot(x, y, x, y1);
          if(spot == 0)
          {
            ec2++;
          }
          else
          {
            if(spot == 2) ec2++;
            addLocation(player, piece, x, y1);
          }
        }
        if(x2 >= 0 && ec3 == 0)
        {
          int spot = landableSpot(x, y, x2, y);
          if(spot == 0)
          {
            ec3++;
          }
          else
          {
            if(spot == 2) ec3++;
            addLocation(player, piece, x2, y);
          }
        }
        if(y2 >= 0 && ec4 == 0)
        {
          int spot = landableSpot(x, y, x, y2);
          if(spot == 0)
          {
            ec4++;
          }
          else
          {
            if(spot == 2) ec4++;
            addLocation(player, piece, x, y2);
          }
        }
        if(x1 < 8 && y1 < 8 && ec5 == 0)
        {
          int spot = landableSpot(x, y, x1, y1);
          if(spot == 0)
          {
            ec5++;
          }
          else
          {
            if(spot == 2) ec5++;
            addLocation(player, piece, x1, y1);
          }
        }
        if(x1 < 8 && y2 >= 0 && ec6 == 0)
        {
          int spot = landableSpot(x, y, x1, y2);
          if(spot == 0)
          {
            ec6++;
          }
          else
          {
            if(spot == 2) ec6++;
            addLocation(player, piece, x1, y2);
          }
        }
        if(x2 >= 0 && y1 < 8 && ec7 == 0)
        {
          int spot = landableSpot(x, y, x2, y1);
          if(spot == 0)
          {
            ec7++;
          }
          else
          {
            if(spot == 2) ec7++;
            addLocation(player, piece, x2, y1);
          }
        }
        if(x2 >= 0 && y2 >= 0 && ec8 == 0)
        {
          int spot = landableSpot(x, y, x2, y2);
          if(spot == 0)
          {
            ec8++;
          }
          else
          {
            if(spot == 2) ec8++;
            addLocation(player, piece, x2, y2);
          }
        }
      }
    }
    else if(piece == 15)
    {
      int i = 0;
      int j = 0;
      for(i = -1; i <= 1; i++)
      {
        for(j = -1; j <= 1; j++)
        {
          int tx = x + i;
          int ty = y + j;
          if(tx >= 0 && tx < 8 && ty >= 0 && ty < 8){
            if(landableSpot(x, y, tx, ty) != 0) addLocation(player, piece, tx, ty);
          }
        }
      }
    }
  }
}

int canMove(int sx, int sy, int dx, int dy)
{
  return 0;
}

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
      row[0] = 'R' + ((i == 7) ? white->off : black->off);
      row[1] = 'N' + ((i == 7) ? white->off : black->off);
      row[2] = 'B' + ((i == 7) ? white->off : black->off);
      row[3] = 'K' + ((i == 7) ? white->off : black->off);
      row[4] = 'Q' + ((i == 7) ? white->off : black->off);
      row[5] = 'B' + ((i == 7) ? white->off : black->off);
      row[6] = 'N' + ((i == 7) ? white->off : black->off);
      row[7] = 'R' + ((i == 7) ? white->off : black->off);
    }
    else if(i == 1 || i == 6)
    {
      int j = 0;
      for(j = 0; j < 8; j++)
        row[j] = 'P' + ((i == 6) ? white->off : black->off);
    }
    board[i] = row;
  }
}

void printBoard()
{
  int i = 0;
  int j = 0;
  printf("%s  | A| B| C| D| E| F| G| H|  \n", other_color);
  for(i = 0; i < 8; i++)
  {
    printf("--+--+--+--+--+--+--+--+--+--\n");
    printf(" %i|", i + 1);
    for(j = 0; j < 8; j++)
    {
      int off = (board[i][j] > 96) ? white->off : black->off;
      printf(" %s%c%s|", (board[i][j] > 96) ? white->color : black->color,
                         (board[i][j] > 96) ? board[i][j]/* - off*/ : board[i][j],
                         other_color);
    }
    printf("%i \n", i + 1);
  }
  printf("--+--+--+--+--+--+--+--+--+--\n");
  printf("  | A| B| C| D| E| F| G| H|  %s\n", reset_color);
}

void clearScreen()
{
  //clear the screen
  printf("\x1B[2J");
  //move cursor to top corner of screen
  printf("\x1B[1;1H");
}

void printMessage(const char* message)
{
  //print message in red
  printf("%s%s%s", message_color, message, reset_color);
}

void printLocations()
{
  //if(loc_index == 0)
  //{
  //  printf("\n");
  //  return;
  //}
  //printf("%s", (num_of_turns % 2 == 0) ? white_piece_color : black_piece_color);
  //int i = 0;
  //for(i = 0; i < loc_index - 1; i++)
   // printf("%s, ", location_buffer[i]);
  //printf("%s", location_buffer[i]);
  //printf("\n%s", reset_color);
}

void printTurns()
{
  printf("%sThere have been %i turns (%s)%s\n", (num_of_turns % 2 == 0) ? white->color : black->color,
                                                num_of_turns,
                                                (num_of_turns % 2 == 0) ? white->name : black->name,
                                                reset_color);
}

void errorMessage()
{
  clearScreen();
  printMessage("Unrecognized SAN\n");
  printTurns();
  printBoard();
}

void moveMessage(int src, int dest)
{
  //clearScreen();
 // printMessage("Moved Piece: ");
  //printMessage(location[0]);
  //printMessage(" to ");
  //printMessage(location[1]);
  //printMessage("\n");
  //printTurns();
  //printBoard();
}

void movePiece(Player* player, int src, int dest)
{
  //if pawn moves for the first time and moves 2 squares set enpassant then unset it next turn
}

void helpMessage(Player* player, int piece)
{
  //clearScreen();
  //printMessage("Help on Piece: ");
  //printMessage(location[0]);
  //printMessage((board[sy][sx] < 97) ? ((board[sy][sx] == ' ') ? "[Empty]\n" : "[Black]\n") : "[White]\n");
  //printLocations();
  //printBoard();
}

void help(Player* player, int x, int y)
{

}

int main(int argc, char** argv)
{
  char white_name[BUFFER_LEN + 1];
  char black_name[BUFFER_LEN + 1];
  if(argc != 3)
  {
    printf("Name of Player 1 (White): ");
    scanf("%s", white_name);
    printf("Name of Player 2 (Black): ");
    scanf("%s", black_name);
    getchar();
  }
  else
  {
    memcpy((void*)white_name, argv[1], sizeof(char) * BUFFER_LEN);
    memcpy((void*)black_name, argv[2], sizeof(char) * BUFFER_LEN);
  }

  white = buildPlayer(1, white_name, "\x1B[97m", 32);
  black = buildPlayer(0, black_name, "\x1B[90m", 0);

  clearBoard();

  findMoves(white);
  findMoves(black);

  clearScreen();
  printMessage("Welcome ");
  printMessage(white_name);
  printMessage(" and ");
  printMessage(black_name);
  printMessage("\n");
  printMessage(white_name);
  printMessage(" goes first!\n");
  printBoard();

  char input[BUFFER_LEN + 1];
  memset(input, 0, BUFFER_LEN + 1);
  while(1)
  {
    //get input
    fgets(input, BUFFER_LEN, stdin);
    //process input
    if(strcmp(input, "quit") == 0)
    {
      break;//break out of loop
    }
    else //we might have a command
    {
      San* command = parse(input);
      clearScreen();
      printf("%i, %c%c, %c, %c%c, %i", command->piece, command->src[0], command->src[1], command->cap, command->dest[0], command->dest[1], command->suffix);
      //printMessage("Moved Piece: ");
      printTurns();
      printBoard();
    }
  }
  freePlayer(white);
  freePlayer(black);
  return 0;
}
