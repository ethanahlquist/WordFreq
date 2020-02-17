#include <stdlib.h>
#include "hashTable.h"
#include "getWord.h"

int compareEntry(const void *e1, const void *e2);

/* Prototype of the function you must write for this exercise.
 *
 * This function uses the C Standard Library qsort function to sort the
 * provided array HTEntry structures.
 *
 * Note that you will also need to write a compare function to provide to
 * qsort - "man qsort" for details - that you will be calling in this
 * function.
 */
void qsortHTEntries(HTEntry *entries, int numberOfEntries) {

   qsort(entries, numberOfEntries, sizeof(HTEntry), compareEntry);
}

int compareEntry(const void *e1, const void *e2) {

   int f_diff = ((((HTEntry*)e2)->frequency - ((HTEntry*)e1)->frequency));
   if(0 != f_diff)
      return f_diff;
   else
      return compareWord((Word*)((HTEntry*)e1)->data,
         (Word*)((HTEntry*)e2)->data);
}

