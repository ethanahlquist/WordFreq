#include "linkedList.h"
#include "myMacros.h"
#include "hashTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int findNode(ListNode ** nodePointer, void *data, FNCompare compare){

   if(*nodePointer == NULL)
      return 0;

   while((*compare)(data, (*nodePointer)->entry->data)) {
      if((*nodePointer)->next == NULL)
         return 2;
      *nodePointer = (*nodePointer)->next;
   }
   return 1;
}

void addListToEntryList(HTEntry* entryArray, ListNode * head, unsigned *size)
{
   ListNode* nodePointer;
   while(head != NULL){
      nodePointer = head;
      head = head->next;
      entryArray[((*size)++)] = *(nodePointer->entry);
   }
}

void destroyNode(ListNode * node, FNDestroy destroy) {

   if(destroy != NULL)
      destroy(((node->entry)->data));
   free((node->entry->data));
   free(node->entry);
   free(node);
}

void destroyList(ListNode *head, FNDestroy destroy) {

   ListNode* nodePointer;
   while(head != NULL){
      nodePointer = head;
      head = head->next;
      destroyNode(nodePointer, destroy);
   }
}

ListNode *createListNode(void *data) {

   HTEntry *newEntry;
   ListNode *newNode;
   MY_CALLOC(newNode, 1, ListNode);
   MY_CALLOC(newEntry, 1, HTEntry);
   newNode->entry = newEntry;
   newEntry->data = data;
   newEntry->frequency = 1;
   return newNode;
}

unsigned addListEntry(ListNode **head, void *data, FNCompare compare){

   int Flag;
   ListNode *nodePointer;
   nodePointer = *head;

   Flag = findNode(&nodePointer, data, compare);

   if(Flag == 0){
      *head = createListNode(data);
      nodePointer = *head;
   }
   else if(Flag == 1)
      nodePointer->entry->frequency ++;
   else if(Flag == 2){
      nodePointer->next = createListNode(data);
      return 1;
   }

   return nodePointer->entry->frequency;
}

void addHead(ListNode **list, ListNode *newNode) {
   newNode->next = *list;
   *list = newNode;
}

