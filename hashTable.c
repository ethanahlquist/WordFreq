#include <limits.h>
#include <assert.h>
#include "hashTable.h"
#include "linkedList.h"
#include "myMacros.h"

#define NUM(A) (sizeof(A) / sizeof(*A))

/*
 *{{{ Helper Declarations
 */
void assertHtCreate(unsigned *, int, float);
void createDeepCopy(HashTable *, HTFunctions *, int, float, unsigned []);
void metricList(ListNode *, HTMetrics *);
void rehashTable(HashTable *);
/* }}}
 */

/*
 * {{{ Structures -
 *
 * typedef struct
 * {
 *    void *data;
 *    unsigned frequency;
 * } HTEntry;
 *
 * typedef struct
 * {
 *    unsigned numberOfChains;
 *    unsigned maxChainLength;
 *    float avgChainLength;
 * } HTMetrics;
 *
 * typedef struct
 * {
 *    FNHash hash;
 *    FNCompare compare;
 *    FNDestroy destroy;
 * } HTFunctions;
 *
 * }}}
 */

/*
 * {{{ Funcs -
 *
 *    FNHash: Calculates and returns the hash value (not index!) for the
 *       specified data.
 *
 *    FNCompare: Compares two data items to determine data1 is less-than,
 *       equal-to, or greater than data2. The function returns a negative,
 *       zero, or positive value, respectively.
 *
 *    FNDestroy: May be NULL. This function must be provided (not NULL) when
 *       the data being stored in the hash table, itself, has additional
 *       dynamically allocated memory (sub-allocations) that only the owner of
 *       the data knows about and can free, otherwise it should be NULL. When
 *       provided, this function frees all such sub-allocations BUT does NOT
 *       free the primary data itself as that is done by htDestroy. This
 *       division of labor avoids the error that occurs if free is called more
 *       than once on the same memory allocation.
 *
 * typedef unsigned (*FNHash)(const void *data);
 * typedef int (*FNCompare)(const void *data1, const void *data2);
 * typedef void (*FNDestroy)(const void *data);
 * }}}
 */

/*
 * {{{ htCreate -
 * Description: Creates a new hash table as specified.
 *
 * Notes:
 *   1. The function asserts (man 3 assert) if numSizes is not 1 or more.
 *   2. The function asserts (man 3 assert) if any of the sizes are not
 *      greater than the immediately preceding size.
 *   3. The function asserts (man 3 assert) If the load factor is not greater
 *      than 0.0 and less than or equal to 1.0.
 *   4. You must make a deep copy of the _functions_ pointers and _sizes_
 *      array since you don't know where the caller allocated them - they
 *      could be on the stack and you don't want to point there - that's a
 *      potential and likely _unlucky bug_ waiting to happen!
 *
 * Parameters:
 *    functions: A structure of function pointers to the data-specific
 *       functions required by the hash table.
 *    sizes: An array of hash table sizes. Must always be at least one size
 *       greater than zero specified, more if rehashing is desired. Prime
 *       numbers are best but not required/checked for. That being said, you
 *       should use prime numbers!
 *    numSizes: The number of values in the sizes array.
 *    rehashLoadFactor: The load factor to rehash at. Should be a value greater
 *       than 0.0 and less than or equal to 1.0. A value of 1.0 means
 *       "do not rehash".
 *
 * Return: A pointer to an anonymous (file-local) structure representing a
 *         hash table.
 * }}}
 */
void* htCreate(
   HTFunctions *functions,
   unsigned sizes[],
   int numSizes,
   float rehashLoadFactor)
{

   HashTable *ht = calloc(1, sizeof(HashTable));
   assert(ht != NULL);

   assertHtCreate(sizes, numSizes, rehashLoadFactor);

   createDeepCopy(ht, functions, numSizes, rehashLoadFactor, sizes);

   return ht;
}

void assertHtCreate(unsigned sizes[], int numSizes, float rehashLoadFactor)
{
   int i;

   /* core01 & core02 */
   assert(numSizes > 0);

   /* core03 */
   assert(sizes[0] > 0);

   /* core04 & core05 */
   for(i = 1; i < numSizes; i++)
      assert(sizes[i-1] < sizes[i]);

   /* core06  & core07 & core08 */
   assert( 0.0 < rehashLoadFactor && rehashLoadFactor <= 1.0);
}

void createDeepCopy(HashTable *ht, HTFunctions *functions, int numSizes,
   float rehashFactor, unsigned sizes[]) {

   int i;
   MY_MALLOC(ht->sizes, numSizes * sizeof(unsigned));

   ht->functions.hash = functions->hash;
   ht->functions.compare = functions->compare;
   ht->functions.destroy = functions->destroy;

   ht->numSizes = numSizes;
   ht->rehashFactor = rehashFactor;

   for(i = 0; i < numSizes; i++)
      ht->sizes[i] = sizes[i];

   ht->sizeIndex = 0;

   MY_CALLOC(ht->actualHT, ht->sizes[ht->sizeIndex], sizeof(ListNode *));
}

/*
 *{{{ htDestroy
 * Description: Frees all of the dynamically allocated memory allocated by the
 *    hash table itself as well as the all of the data object added to the
 *    hash table by the user. Note that when a destroy function was provided to
 *    htCreate, it is called on each data object added by the user prior to
 *    freeing that object.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: None
 *
 * int i;
 * for(i = 0; i < CURRENT_SIZE(pt); i++, pt++)
 *   pt->functions.destroy(pt);
 *
 * }}}
 */
void htDestroy(void *ht)
{
   int i;
   HashTable * pt = (HashTable *)ht;
   ListNode ** htPointer = pt->actualHT;

   for(i = 0; i < CURRENT_SIZE(pt); i++, htPointer++)
      destroyList(*htPointer, (&pt->functions)->destroy);

   free(pt->sizes);
   free(pt->actualHT);
   free(pt);
}

/*
 * {{{ htAdd -
 * Description: Adds a shallow copy of the data to the hash table. The data
 *    being added MUST BE dynamically allocated because it will be freed
 *    when the hash table is destroyed!
 *
 * Notes:
 *    1. The function is expected to have O(1) performance.
 *    2. The function asserts (man 3 assert) if data is NULL.
 *    3. The function rehashes to the next size when:
 *          1. The rehash load factor is NOT 1.0, a value of 1.0 indicates
 *             rehashing is not desired.
 *          2. And there is a next size. Otherwise continue with the current
 *             size.
 *          3. And the ratio of unique entries TO the current hash table size
 *             (BEFORE adding the new data) exceeds the rehash load factor.
 *    4. When the data being added is a duplicate, the original entry is kept
 *       in the hash table AND the caller is responsible for freeing the
 *       duplicate.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    data: The data to add.
 *
 * Return: The frequency of the data in the hash table. A value of 1 means it
 *    is a new and unique entry, values greater than 1 mean it is a duplicate
 *    with the indicated frequency.
 * }}}
 */
unsigned htAdd(void *ht, void *data)
{
   unsigned freq;
   long int hashIndex = 0;
   HashTable *pt = (HashTable *)ht;

   assert(data != NULL);

   if(pt->rehashFactor < ((float)htUniqueEntries(pt)/(float)CURRENT_SIZE(pt))
      && ((pt->sizeIndex +1) < pt->numSizes) )
      rehashTable(pt);

   hashIndex = (pt->functions.hash(data) % CURRENT_SIZE(pt));
   pt->totalEntries++;

   if( 1  == (freq = addListEntry(&(pt->actualHT[hashIndex]),
      data, (&pt->functions)->compare)))
      pt->uniqueEntries ++;
   return freq;
}

void rehashList(ListNode *headPrev, ListNode **newArray,
   unsigned newSize, FNHash hash) {

   ListNode* nodePointer;
   while(headPrev != NULL){
      nodePointer = headPrev;
      headPrev = headPrev->next;
      addHead(&(newArray[hash(nodePointer->entry->data)%newSize]), nodePointer);
   }
}

void rehashTable(HashTable *pt) {

   int i;
   unsigned nextSize = pt->sizes[(pt->sizeIndex)+1];
   ListNode **newArray;
   ListNode **prevArray = pt->actualHT;
   MY_CALLOC(newArray, nextSize, sizeof(ListNode *));

   assert(nextSize != 0);

   for(i = 0; i < CURRENT_SIZE(pt); i++, prevArray++){
      rehashList(*prevArray, newArray, nextSize, (&pt->functions)->hash);
   }
   free(pt->actualHT);
   pt->sizeIndex +=1;
   pt->actualHT = newArray;
}

/*
 * {{{ htLookUp
 * Description: Determines if the data is in the hash table or not.
 *
 * Notes:
 *    1. The function is expected to have O(1) performance.
 *    2. The function asserts (man 3 assert) if data is NULL.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    data: The data to look for.
 *
 * Return: Returns an HTEntry with a shallow copy of the data and its
 *    frequency if found, otherwise NULL data and frequency 0.
 * }}}
 */
HTEntry htLookUp(void *ht, void *data)
{
   long int hashIndex = 0;
   ListNode *nodePointer;
   HTEntry entry;
   HashTable *pt = (HashTable *)ht;

   entry.data = NULL;
   entry.frequency = 0;

   assert(data != NULL);

   hashIndex = (pt->functions.hash(data) % CURRENT_SIZE(pt));
   nodePointer = pt->actualHT[hashIndex];
   if(findNode(&nodePointer, data, (&pt->functions)->compare) == 1)
      return *(nodePointer->entry);
   else
      return entry;
}

/*
 * {{{
 * Description: Returns a dynamically allocated array with shallow copies of
 * all of the hash table entries.
 *
 * Notes:
 *    1. The caller is responsible for freeing the memory for the returned
 *       array BUT MUST NOT free the data in each HTEntry in the array - that
 *       memory is still in use by the hash table! To free the data call
 *       htDestroy when you are completely done with the hash table AND all
 *       arrays returned by this function.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    size: Output parameter updated with the array's size.
 *
 * Return: A dynamically allocated array with all of the hash table entries or
 *    NULL if the hash table is empty (note that free can be called on NULL).
 * }}}
 */
HTEntry* htToArray(void *ht, unsigned *size)
{
   int i;
   HashTable * pt = (HashTable *)ht;
   ListNode ** htPointer = pt->actualHT;
   HTEntry* entryArray;

   *size = 0;
   if(htUniqueEntries(ht) == 0)
      return NULL;

   MY_MALLOC(entryArray, htUniqueEntries(ht) * sizeof(HTEntry));

   for(i = 0; i < CURRENT_SIZE(pt); i++, htPointer++)
      addListToEntryList(entryArray, *htPointer, size);

   return entryArray;
}

/*
 * {{{
 * Description: Reports the current capacity of the hash table.
 *
 * Notes:
 *    1. The function is expected to have O(1) performance.
 *    2. This function should always return one of the sizes passed to htCreate.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The current capacity of the hash table.
 * }}}
 */
unsigned htCapacity(void *ht)
{
   return (unsigned)(CAST_HT(ht)->sizes[CAST_HT(ht)->sizeIndex]);
}

/*
 * {{{
 * Description: Returns the number of unique entries in the hash table.
 *
 * Notes:
 *    1. The function is expected to have O(1) performance which implies it must
 *       be stored and maintained by the hash table as data is added - NOT
 *       calculated when this function is called.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The number of unique entries in the hash table.
 * }}}
 */
unsigned htUniqueEntries(void *ht)
{
   return CAST_HT(ht)->uniqueEntries;
}

/*
 * {{{
 * Description: Returns the total number of data objects added to the hash
 *    table which is equivalent to the sum of the frequencies of all entries
 *    in the hash table (see Notes, below, for important information about
 *    the expected performance of this function).
 *
 * Notes:
 *    1. The function is expected to have O(1) performance which implies it must
 *       be stored and maintained by the hash table as data is added - NOT
 *       calculated when this function is called.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The sum of the frequencies of all entries in the hash table.
 * }}}
 */
unsigned htTotalEntries(void *ht)
{
   return CAST_HT(ht)->totalEntries;
}

/*
 * {{{
 * Description: Returns various metrics on the hash table to aid in performance
 *    tuning of CPU and memory usage for a particular problem domain.
 *
 * Notes:
 *    1. Because this function will only be used for performance tuning and not
 *       during actual use of the hash table it DOES NOT need to have O(1)
 *       performance. FYI: O(N) performance is expected.
 *    2. The number of chains is a count of the non-empty chains in the
 *       hash table.
 *    3. The maximum chain length is the length of the longest chain in the
 *       hash table.
 *    4. The average chain length is the sum of the length of all chains
 *       in the hash table divided by the number of non-zero length chains.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: An HTMetric struct with the current metrics for the specified hash
 *    table.
 * }}}
 */
HTMetrics htMetrics(void *ht)
{
   int i;
   HTMetrics metrics;
   HashTable *pt = (HashTable *)ht;

   ListNode ** htPointer = pt->actualHT;

   metrics.numberOfChains = 0;
   metrics.maxChainLength = 0;

   for(i = 0; i < CURRENT_SIZE(pt); i++, htPointer++)
      metricList(*htPointer, &metrics);

   metrics.avgChainLength = ((float) htUniqueEntries(ht) /
      (float) metrics.numberOfChains);

   return metrics;
}

void metricList(ListNode *headPrev, HTMetrics *metrics) {

   unsigned chainLength = 0;

   if(headPrev != NULL)
      (metrics->numberOfChains)++;
   while(headPrev != NULL){
      headPrev = headPrev->next;
      chainLength ++;
   }
   metrics->maxChainLength = MAX(chainLength, metrics->maxChainLength);
}
