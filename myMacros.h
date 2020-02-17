#ifndef MYMACROS_h
#define MYMACROS_h

#include "hashTable.h"
#include "linkedList.h"

typedef struct {

   HTFunctions functions;
   int numSizes;
   float rehashFactor;
   unsigned *sizes;

   int sizeIndex;
   ListNode **actualHT;
   unsigned totalEntries;
   unsigned uniqueEntries;


} HashTable;

#define MIN(A,B) (((A) < (B)) ? (A):(B))
#define MAX(A,B) (((A) > (B)) ? (A):(B))

#define CURRENT_SIZE(ht) (unsigned)(ht->sizes[ht->sizeIndex])
#define CAST_HT(ht) ((HashTable *)ht)

#define MY_MALLOC(_ptr,_size) \
   do { \
       if (NULL == ((_ptr) = malloc(_size))) { \
              fprintf(stderr, "Failed to allocate memory, in %s at line %d.\n"\
               ,__FILE__, __LINE__); \
              exit(EXIT_FAILURE);\
           } \
   } while(0)

/*#undef MY_MALLOC*/

#define MY_CALLOC(_ptr,_num,_type) \
   do { \
       if (NULL == ((_ptr) = calloc(_num, sizeof(_type)))) { \
              fprintf(stderr, "Failed to allocate memory, in %s at line %d.\n"\
               ,__FILE__, __LINE__); \
              exit(EXIT_FAILURE);\
           } \
   } while(0)

/*#undef MY_CALLOC*/
#endif

