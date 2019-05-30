#include "sanparser.h"

San* parse(char* input)
{
  if(input == NULL) return NULL;
  San* result = (San*)malloc(sizeof(struct san));
  memset(result, 0, sizeof(struct san));

  int suf = 0;
  char* start = input;

  switch(start[0])
  {
    case 'N':
      result->piece = KNIGHT;
      start++;
      break;
    case 'B':
      result->piece = BISHOP;
      start++;
      break;
    case 'R':
      result->piece = ROOK;
      start++;
      break;
    case 'Q':
      result->piece = QUEEN;
      start++;
      break;
    case 'K':
      result->piece = KING;
      start++;
      break;
    default:
      if((start[0] <= 104 && start[0] >= 97) ||
         (start[0] <= 56 && start[0] >= 49) ||
         (start[0] == 'x'))
      {
        result->piece = PAWN;
        start++;
      }
      else
      {
        suf = 1;
      }
      break;
  }

  if(suf == 0)
  {
    int startLength = strlen(start);
    int i = 0;
    for(i = startLength - 2; i >= 0 ; i--)
      if(start[i] <= 104 && start[i] >= 97 && start[i+1] <= 56 && start[i+1] >= 49) break;

    if(i == 0)//we either have a destination or we have a help command
    {
      if(startLength == 3)//if we have a question mark after the only location we found it is a help command
      {
        if(start[2] == '?')//need the nesting to make sure we can access the third element
        {
          memcpy(result->src, start, sizeof(char) * 2);
          result->cap = '?';
          return result;
        }
      }
      //otherwise this is for sure a destination and the rest of the stuff is suffix stuff
      memcpy(result->dest, start, sizeof(char) * 2);
      start += 2;
    }
    else if(i < startLength)//we have stuff at the beginning to take care of
    {
      if(start[i - 1] == 'x') //if the last character of the beginning is a capture
      {
        result->cap = 'x';
      }
      if(i > ((result->cap == 'x') ? 3 : 2))//not possible unless malformed input
      {
        free(result);
        return NULL;
      }
      //otherwise we have a src at start
      memcpy(result->src, start, sizeof(char) * ((result->cap == 'x') ? i - 1 : i));
      //now grab dest and suffix
      start += i;
      memcpy(result->dest, start, sizeof(char) * 2);
      start += 2;
    }
    else//if i is at -1 we have a malformed input
    {
      free(result);
      return NULL;
    }
  }
  //finish parsing the suffix
  return result;
}
