#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "getWord.h"

#define MIN(A,B) (((A) < (B)) ? (A):(B))

void addToWord(int, Byte **, unsigned *, int *, unsigned *);
void *my_malloc(unsigned);
void *my_realloc(void *, size_t);

int getWord(FILE *file, Byte **word, unsigned *wordLength, int *hasPrintable)
{
   int ch;
   unsigned allocSize = 8;
   *hasPrintable = 0;
   *wordLength = 0;

   *word = my_malloc(allocSize);

   while( EOF != (ch = fgetc(file)))
      if(!isspace(ch))
         break;
   do{
      if(isspace(ch) || ch == EOF ) {
         void * tmp = realloc(*word, *wordLength);
         if (NULL == tmp)
            free(tmp);
         else
            (*word) = tmp;
         return ch;
      }
      else
         addToWord(ch, word, wordLength, hasPrintable, &allocSize);

      ch = fgetc(file);
   } while(1);
}

void *my_malloc(unsigned size){

   void * pointer;
   if( NULL == (pointer = malloc(size) ) ){
      fprintf(stderr, "Cannot allocate memory\n");
      exit(EXIT_FAILURE);
   }
   return pointer;
}

void * my_realloc(void * ptr, size_t size){

   void * pointer;
   if( NULL == (pointer = realloc(ptr, size) ) ){
      fprintf(stderr, "Cannot allocate memory\n");
      exit(EXIT_FAILURE);
   }
   return pointer;
}

void addToWord(int ch, Byte ** word, unsigned *wordLength,
   int *hasPrintable, unsigned *allocSize)
{
   /* Double the allocation size, using bitshift operator */
   if(*wordLength >= *allocSize)
      *word = my_realloc(*word, ((*allocSize) <<= 1) );

   /* Acts like a latch */
   if(isprint(ch))
      *hasPrintable = 1;

   (*word)[(*wordLength)++] = tolower(ch);
}

/* MODIFICATION FROM:
 *
 * https://stackoverflow.com/questions/7700400/whats-a-good-hash-function-
 * for-english-words?noredirect=1
 *
 */
unsigned hashWord(const void * word)
{
   unsigned long hash = 5381;
   Byte *startByte = ((Word *)word)->bytes;
   int i;

   for(i = 0; i < ((Word *)word)->length; i++)
      hash = ((hash << 5) + hash) + (*(((Word *)word)->bytes)++);

   ((Word *)word)->bytes = startByte;

   return hash;
}

void destroyWord(const void *word)
{
   free(((Word *)word)->bytes);
}

int compareWord(const void *w1, const void *w2)
{
   int size;
   int L1 = ((Word*)w1)->length;
   int L2 = ((Word*)w2)->length;
   int min = MIN(L1, L2);

   if((size = memcmp(((Word*)w1)->bytes, ((Word*)w2)->bytes, min)))
      return size;
   return (L1 - L2);
}
