#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "hashTable.h"

typedef struct node
{
   HTEntry *entry;
   struct node *next;
} ListNode;

#define NODE_ENTRY(NODE) ((HTEntry *)(NODE)->entry)
#define NODE_ENTR(NODE) ((HTEntry *)(&(NODE->entry)))
/*
 * Adds the value to the front of the list. Has O(1) performance.
 *
 * Parameter list: The head of the list, NULL if empty.
 * Parameter value: The value to add to the beginning of the list.
 *
 * Return: The head of the list.
 */

/*
 * Adds the value to the end of the list. Has O(N) performance.
 *
 * Parameter list: The head of the list, NULL if empty.
 * Parameter value: The value to add to the end of the list.
 *
 * Return: The head of the list.
 */
ListNode* addTail(ListNode *list, void *entry);

/*
 * Deletes the node at the specified index. Indexes are zero-base. Reports
 * an error in the index is out-out-bounds (see reference solution behavior
 * for specific text you must match). Has O(N) performance.
 *
 * Parameter list: The head of the list, NULL if empty.
 * Parameter index: The zero-based index of the value to remove from the list.
 *
 * Return: The head of the list.
 */
ListNode* deleteNode(ListNode *list, int index);

/*
 * Prints the values, if any, in the list from beginning to end. Has O(N)
 * performance.
 *
 * Parameter list: The head of the list, NULL if empty.
 *
 * Return: Nothing.
 */
void printList(ListNode *list);

unsigned addListEntry(ListNode **head, void *data, FNCompare compare);

int findNode(ListNode ** nodePointer, void *data, FNCompare compare);

void destroyList(ListNode *head, FNDestroy destroy);

void addHead(ListNode **list, ListNode *newNode);

void addListToEntryList(HTEntry*, ListNode *, unsigned *);
#endif
