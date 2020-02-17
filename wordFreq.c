#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashTable.h"
#include "getWord.h"
#include "qsortHTEntries.h"
#include "myMacros.h"

static FILE* openFile(const char *fname, void *ht)
{
   FILE* file = fopen(fname, "r");

   if (file == NULL)
   {
      fprintf(stderr, "wf: %s: ", fname);
      perror(NULL);
      htDestroy(ht);
      exit(EXIT_FAILURE);
   }

   return file;
}

void flagCases(char *arg, int *numberOfWords)
{
   if(!strncmp(arg, "-n", 2))
      sscanf(arg, "%*c%*c%d", numberOfWords);
   else {
      fprintf(stderr, "Usage: wf [-nX] [file...]\n");
      exit(EXIT_FAILURE);
   }
}

static void parseFlags(int argc, char *argv[], int *numberOfWords)
{
   int i;
   for(i = 0; i < argc; i++)
      if(!strncmp(argv[i], "-", 1))
         flagCases(argv[i], numberOfWords);
}

void addWordToTable(void *ht, Byte **word, unsigned wordLength){

   Word *wordStruct;
   MY_MALLOC(wordStruct, sizeof(Word));

   wordStruct->length = wordLength;
   wordStruct->bytes = (*word);

   if( 1 < htAdd(ht, wordStruct) ){
      free(wordStruct->bytes);
      free(wordStruct);
   }
}

void getWordSingleFile(char *arg, void *ht)
{
   Byte *word;
   unsigned wordLength = 0;
   int hasPrintable;
   FILE *file;

   if(arg == NULL)
      file = stdin;
   else
      file = openFile(arg, ht);

   while(EOF != getWord(file, &word, &wordLength, &hasPrintable)){
      if(hasPrintable)
         addWordToTable(ht, &word, wordLength);
      else
         free(word);
   }

   if(hasPrintable)
      addWordToTable(ht, &word, wordLength);

   fclose(file);
}

void getWordAllFiles(void *ht, int argc, char *argv[])
{
   int i;
   for(i = 1; i < argc; i++)
      if(strncmp(argv[i], "-", 1))
         getWordSingleFile(argv[i], ht);
   /* read from stdin */
   if(i == 1)
      getWordSingleFile(NULL, ht);
}

void printWords(void *ht, HTEntry *entries, int size)
{
   int i, j;
   unsigned length;

   printf("%u unique words found in %u total words\n", htUniqueEntries(ht),
      htTotalEntries(ht));

   /* resize so no seg fault */
   if(size > htUniqueEntries(ht))
      size = htUniqueEntries(ht);

   for (i = 0; i < size; i++)
   {
      length = ((Word*)entries[i].data)->length;
      printf("%10u - ", entries[i].frequency);

      for (j = 0; j < length && j < 30; j++) {
         if( isprint( ((Word*)entries[i].data)->bytes[j] ))
            printf("%c", ((Word*)entries[i].data)->bytes[j]);
         else
            printf(".");
      }
      if( length > j )
         printf("...");

      printf("\n");
   }
}

int main(int argc, char *argv[]) {

   HTFunctions funcs = {hashWord, compareWord, destroyWord};
   int numSizes = 26;
   int numberOfWords = 10;
   unsigned size;
   unsigned sizes[] = {
      53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
      49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
      12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
      805306457, 1610612741, 4294967295
   };

   void *ht = htCreate(&funcs, sizes, numSizes, 1);
   HTEntry *entries;

   parseFlags(argc, argv, &numberOfWords);

   getWordAllFiles(ht, argc, argv);

   entries = htToArray(ht, &size);

   qsortHTEntries(entries, size);

   printWords(ht, entries, numberOfWords);

   free(entries);
   htDestroy(ht);

   return EXIT_SUCCESS;
}
