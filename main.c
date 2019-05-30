#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define BUFFER_LEN 255

#define BISHOP 'B'
#define KING   'K'
#define KNIGHT 'N'
#define PAWN   'P'
#define QUEEN  'Q'
#define ROOK   'R'

int white_piece_off = 32;
const char* white_piece_color = "\x1B[97m";
int black_piece_off = 0;
const char* black_piece_color = "\x1B[90m";

const char* message_color = "\x1B[91m";
const char* other_color = "\x1B[36m";
const char* reset_color = "\x1B[0m";

char** board = (void*)0;

typedef struct player{
  char* name;
  char* color;
  int off;
  int* pieces; //length 16 array with the locations of each piece. PPPPPPPPRRNNBBQK
  int* moves_len; //length 16 array wwith the lengths of each piece's move array (if -1 the moves array needs to be updated)
  int** moves; //length 16 array with the moves a piece can make
  int* captured; //length 6 array of the number of pieces PNBRQK
}Player;

Player* buildPlayer(int white, char* name, char* color, int off)
{
  Player* p = (Player*)malloc(sizeof(struct player));
  memset(p, 0, sizeof(struct player));

  int namelength = strlen(name);
  p->name = (char*)malloc(sizeof(char) * (namelength + 1));
  memset(p->name, 0, sizeof(char) * (namelength + 1))
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

}

char* location_buffer[64];
int loc_x = -1;
int loc_y = -1;
int loc_index = 0; //index into the location buffer

int num_of_turns = 0;

void clearLocationBuffer(Player*)
{
  loc_index = 0;
  loc_x = -1;
  loc_y = -1;
  int i = 0;
  for(i = 0; i < 64; i++)
  {
    if(location_buffer[i] == NULL) break;
    free(location_buffer[i]);
    location_buffer[i] = NULL;
  }
}

void addLocation(int x, int y)
{
  char* loc = (char*)malloc(sizeof(char) * 3);
  loc[0] = 'A' + x;
  loc[1] = '1' + y;
  loc[2] = '\0';
  location_buffer[loc_index++] = loc;
}

int landableSpot(char piece, int dx, int dy)
{
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

void findMoves(int x, int y)
{
  //(0, 0) is top left or A1 of the board
  if(loc_x == x && loc_y == y) return;
  clearLocationBuffer();
  char piece = board[y][x];
  loc_x = x;
  loc_y = y;
  if(piece == BISHOP || piece == BISHOP + white_piece_off)
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
        int spot = landableSpot(piece, x1, y1);
        if(spot == 0)
        {
          ec1++;
        }
        else
        {
          if(spot == 2) ec1++;
          addLocation(x1, y1);
        }
      }
      if(x1 < 8 && y2 >= 0 && ec2 == 0)
      {
        int spot = landableSpot(piece, x1, y2);
        if(spot == 0)
        {
          ec2++;
        }
        else
        {
          if(spot == 2) ec2++;
          addLocation(x1, y2);
        }
      }
      if(x2 >= 0 && y1 < 8 && ec3 == 0)
      {
        int spot = landableSpot(piece, x2, y1);
        if(spot == 0)
        {
          ec3++;
        }
        else
        {
          if(spot == 2) ec3++;
          addLocation(x2, y1);
        }
      }
      if(x2 >= 0 && y2 >= 0 && ec4 == 0)
      {
        int spot = landableSpot(piece, x2, y2);
        if(spot == 0)
        {
          ec4++;
        }
        else
        {
          if(spot == 2) ec4++;
          addLocation(x2, y2);
        }
      }
    }
  }
  else if(piece == KING || piece == KING + white_piece_off)
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
          if(landableSpot(piece, tx, ty) != 0) addLocation(tx, ty);
        }
      }
    }
  }
  else if(piece == KNIGHT || piece == KNIGHT + white_piece_off)
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
        if(landableSpot(piece, x1, y2) != 0) addLocation(x1, y2);
      }
      if(y3 < 8){
        if(landableSpot(piece, x1, y3) != 0) addLocation(x1, y2);
      }
    }
    if(x2 >= 0){
      if(y1 >= 0){
        if(landableSpot(piece, x2, y1) != 0) addLocation(x2, y1);
      }
      if(y4 < 8){
        if(landableSpot(piece, x2, y4) != 0) addLocation(x2, y4);
      }
    }
    if(x3 < 8){
      if(y1 >= 0){
        if(landableSpot(piece, x3, y1) != 0) addLocation(x3, y1);
      }
      if(y4 < 8){
        if(landableSpot(piece, x3, y4) != 0) addLocation(x3, y4);
      }
    }
    if(x4 < 8){
      if(y2 >= 0){
        if(landableSpot(piece, x4, y2) != 0) addLocation(x4, y2);
      }
      if(y3 < 8){
        if(landableSpot(piece, x4, y3) != 0) addLocation(x4, y3);
      }
    }
  }
  else if(piece == PAWN || piece == PAWN + white_piece_off)
  {
    if(piece == PAWN)
    {
      int y1 = y + 1;
      if(y1 < 8)
      {
        if(landableSpot(piece, x, y1) == 1) addLocation(x, y1);
        if(x < 7)
          if(landableSpot(piece, x + 1, y1) == 2)
            addLocation(x + 1, y1);
        if(x > 0)
          if(landableSpot(piece, x - 1, y1) == 2)
            addLocation(x - 1, y1);
      }
      if(y == 1)
        if(landableSpot(piece, x, y1 + 1) == 1)
          addLocation(x, y1 + 1);
    }
    else
    {
      int y1 = y - 1;
      if(y1 >= 0)
      {
        if(landableSpot(piece, x, y1) == 1) addLocation(x, y1);
        if(x < 7)
          if(landableSpot(piece, x + 1, y1) == 2)
            addLocation(x + 1, y1);
        if(x > 0)
          if(landableSpot(piece, x - 1, y1) == 2)
            addLocation(x - 1, y1);
      }
      if(y == 6)
        if(landableSpot(piece, x, y1 - 1) == 1)
          addLocation(x, y1 - 1);
    }
  }
  else if(piece == QUEEN || piece == QUEEN + white_piece_off)
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
        int spot = landableSpot(piece, x1, y);
        if(spot == 0)
        {
          ec1++;
        }
        else
        {
          if(spot == 2) ec1++;
          addLocation(x1, y);
        }
      }
      if(y1 >= 0 && ec2 == 0)
      {
        int spot = landableSpot(piece, x, y1);
        if(spot == 0)
        {
          ec2++;
        }
        else
        {
          if(spot == 2) ec2++;
          addLocation(x, y1);
        }
      }
      if(x2 >= 0 && ec3 == 0)
      {
        int spot = landableSpot(piece, x2, y);
        if(spot == 0)
        {
          ec3++;
        }
        else
        {
          if(spot == 2) ec3++;
          addLocation(x2, y);
        }
      }
      if(y2 >= 0 && ec4 == 0)
      {
        int spot = landableSpot(piece, x, y2);
        if(spot == 0)
        {
          ec4++;
        }
        else
        {
          if(spot == 2) ec4++;
          addLocation(x, y2);
        }
      }
      if(x1 < 8 && y1 < 8 && ec5 == 0)
      {
        int spot = landableSpot(piece, x1, y1);
        if(spot == 0)
        {
          ec5++;
        }
        else
        {
          if(spot == 2) ec5++;
          addLocation(x1, y1);
        }
      }
      if(x1 < 8 && y2 >= 0 && ec6 == 0)
      {
        int spot = landableSpot(piece, x1, y2);
        if(spot == 0)
        {
          ec6++;
        }
        else
        {
          if(spot == 2) ec6++;
          addLocation(x1, y2);
        }
      }
      if(x2 >= 0 && y1 < 8 && ec7 == 0)
      {
        int spot = landableSpot(piece, x2, y1);
        if(spot == 0)
        {
          ec7++;
        }
        else
        {
          if(spot == 2) ec7++;
          addLocation(x2, y1);
        }
      }
      if(x2 >= 0 && y2 >= 0 && ec8 == 0)
      {
        int spot = landableSpot(piece, x2, y2);
        if(spot == 0)
        {
          ec8++;
        }
        else
        {
          if(spot == 2) ec8++;
          addLocation(x2, y2);
        }
      }
    }
  }
  else if(piece == ROOK || piece == ROOK + white_piece_off)
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
        int spot = landableSpot(piece, x1, y);
        if(spot == 0)
        {
          ec1++;
        }
        else
        {
          if(spot == 2) ec1++;
          addLocation(x1, y);
        }
      }
      if(y1 >= 0 && ec2 == 0)
      {
        int spot = landableSpot(piece, x, y1);
        if(spot == 0)
        {
          ec2++;
        }
        else
        {
          if(spot == 2) ec2++;
          addLocation(x, y1);
        }
      }
      if(x2 >= 0 && ec3 == 0)
      {
        int spot = landableSpot(piece, x2, y);
        if(spot == 0)
        {
          ec3++;
        }
        else
        {
          if(spot == 2) ec3++;
          addLocation(x2, y);
        }
      }
      if(y2 >= 0 && ec4 == 0)
      {
        int spot = landableSpot(piece, x, y2);
        if(spot == 0)
        {
          ec4++;
        }
        else
        {
          if(spot == 2) ec4++;
          addLocation(x, y2);
        }
      }
    }
  }
}

int canMove(int sx, int sy, int dx, int dy)
{
  if(sx != loc_x || sy != loc_y) findMoves(sx, sy);
  int i = 0;
  for(i = 0; i < loc_index; i++)
  {
    if(location_buffer[i][0] == 'A' + dx && location_buffer[i][1] == '1' + dy) break;
  }
  if(i < loc_index) return 1; //if loop exited before finishing we found a match
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
  printf("%s  | A| B| C| D| E| F| G| H|  \n", other_color);
  for(i = 0; i < 8; i++)
  {
    printf("--+--+--+--+--+--+--+--+--+--\n");
    printf(" %i|", i + 1);
    for(j = 0; j < 8; j++)
    {
      int off = (board[i][j] > 96) ? white_piece_off : black_piece_off;
      printf(" %s%c%s|", (board[i][j] > 96) ? white_piece_color : black_piece_color,
                         (board[i][j] > 96) ? board[i][j] - off : board[i][j],
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
  if(loc_index == 0)
  {
    printf("\n");
    return;
  }
  printf("%s", (num_of_turns % 2 == 0) ? white_piece_color : black_piece_color);
  int i = 0;
  for(i = 0; i < loc_index - 1; i++)
    printf("%s, ", location_buffer[i]);
  printf("%s", location_buffer[i]);
  printf("\n%s", reset_color);
}

void printTurns()
{
  printf("%sThere have been %i turns (%s)%s\n", (num_of_turns % 2 == 0) ? white_piece_color : black_piece_color,
                                                num_of_turns,
                                                (num_of_turns % 2 == 0) ? white_name : black_name,
                                                reset_color);
}

int main(int argc, char** argv)
{
  memset((void*)white_name, 0, sizeof(char) * (BUFFER_LEN + 1));
  memset((void*)black_name, 0, sizeof(char) * (BUFFER_LEN + 1));
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

  clearBoard();
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
    char* command = strtok(input, " ");
    if(strlen(command) > 4) command[4] = '\0'; //chop off command at 4 characters
    char* location[2];
    location[0] = strtok(NULL, " ");
    location[1] = strtok(NULL, "\n");
    if(strcmp(command, "move") == 0)
    {
      //make sure we have a valid location and the piece is yours
      if(location[0] == NULL || location[1] == NULL)
      {
        clearScreen();
        printMessage("The 'move' command requires two (2) parameters: loc and dest\n");
        printTurns();
        printBoard();
        continue;
      }
      //cut off location length
      if(strlen(location[0]) > 2) location[0][2] = '\0';
      if(strlen(location[1]) > 2) location[1][2] = '\0';

      //make sure we have a valid location
      if((((location[0][0] <= 104 && location[0][0] >= 97) ||
           (location[0][0] <= 72  && location[0][0] >= 65)) &&
          (location[0][1]  <= 56  && location[0][1] >= 49)) &&
         (((location[1][0] <= 104 && location[1][0] >= 97) ||
           (location[1][0] <= 72  && location[1][0] >= 65)) &&
          (location[1][1]  <= 56  && location[1][1] >= 49)))
      {
        //valid location
        int sx = (location[0][0] < 97) ? (int)(location[0][0] - 65) : (int)(location[0][0] - 97);
        int sy = (int)(location[0][1] - 49);
        int dx = (location[1][0] < 97) ? (int)(location[1][0] - 65) : (int)(location[1][0] - 97);
        int dy = (int)(location[1][1] - 49);
        //make sure we have our own piece
        char piece = board[sy][sx];
        if((piece < 97)  != (num_of_turns % 2 == 1))
        {
          clearScreen();
          printMessage("You cannot move another player's piece\n");
          printTurns();
          printBoard();
          continue;
        }
        //can the piece move there
        if(canMove(sx, sy, dx, dy) == 0)
        {
          clearScreen();
          printMessage("The piece at ");
          printMessage(location[0]);
          printMessage(" cannot move to location ");
          printMessage(location[1]);
          printMessage("\n");
          printTurns();
          printBoard();
          continue;
        }

        clearLocationBuffer();
        board[sy][sx] = ' ';
        board[dy][dx] = piece;
        //turn increase
        num_of_turns++;
        //print
        clearScreen();
        printMessage("Moved Piece: ");
        printMessage(location[0]);
        printMessage(" to ");
        printMessage(location[1]);
        printMessage("\n");
        printTurns();
        printBoard();
      }
      else
      {
        //error
        clearScreen();
        printMessage("Invalid Parameter: ");
        printMessage(location[0]);
        printMessage(" and/or ");
        printMessage(location[1]);
        printMessage(" aren't valid locations\n");
        printTurns();
        printBoard();
      }
    }
    else if(strcmp(command, "help") == 0)
    {
      if(location[0] == NULL)
      {
        //get help
        clearScreen();
        printMessage("The 'help' command requires one (1) parameter: loc\n\n");
        printBoard();
        continue;
      }

      //cut off location at length 2
      if(strlen(location[0]) > 2) location[0][2] = '\0';

      //make sure we have a valid location
      if(((location[0][0] <= 104 && location[0][0] >= 97) ||
          (location[0][0] <= 72  && location[0][0] >= 65)) &&
          (location[0][1]  <= 56  && location[0][1] >= 49))
      {
        //valid location
        int sx = (location[0][0] < 97) ? (int)(location[0][0] - 65) : (int)(location[0][0] - 97);
        int sy = (int)(location[0][1] - 49);
        findMoves(sx, sy);

        clearScreen();
        printMessage("Help on Piece: ");
        printMessage(location[0]);
        printMessage((board[sy][sx] < 97) ? ((board[sy][sx] == ' ') ? "[Empty]\n" : "[Black]\n") : "[White]\n");
        printLocations();
        printBoard();
      }
      else
      {
        //get help
        clearScreen();
        printMessage("Invalid Parameter: ");
        printMessage(location[0]);
        printMessage(" is not a valid location\n\n");
        printBoard();
      }

    }
    else if(strcmp(command, "quit") == 0)
    {
      break;//break out of loop
    }
    else
    {
      clearScreen();
      printMessage("Unrecognized command: ");
      printMessage(command);
      printTurns();
      printBoard();
    }
  }
  return 0;
}
