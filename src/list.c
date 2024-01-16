/*-------------------------------------------------------------------*/
/* Double-linked List                                                */
/*-------------------------------------------------------------------*/

#include "list.h"
#include <stdlib.h>

/*-------------------------------------------------------------------*/
/* list_create                                                       */
/*-------------------------------------------------------------------*/

void list_create(LIST *list)
{
   list->head = NULL;
   list->tail = NULL;
   list->count = 0;
}

/*-------------------------------------------------------------------*/
/* list_insert_before                                                */
/*-------------------------------------------------------------------*/

void *list_insert_before(LIST *list, void *elem, int elemlen)
{
   LIST_ELEM *prev, *curr, *next, *new;

   curr = (LIST_ELEM *)elem;
   new = malloc(elemlen);
   list->count++;

   if (curr == list->head || curr == NULL) {     /* Insert head elem */
      next = list->head;
      list->head = new;
      new->next = next;
      if (next == NULL)                         /* Old head is NULL? */
         list->tail = new;
      else 
         next->prev = new;

   } else {                                  /* Insert non-head elem */
      prev = curr->prev;
      prev->next = new;
      new->prev = prev;
      curr->prev = new;
      new->next = curr;
   }

   return new;
}

/*-------------------------------------------------------------------*/
/* list_insert_after                                                 */
/*-------------------------------------------------------------------*/

void *list_insert_after(LIST *list, void *elem, int elemlen)
{
   LIST_ELEM *prev, *curr, *next, *new;

   curr = (LIST_ELEM *)elem;
   new = malloc(elemlen);
   list->count++;

   if (curr == list->tail || curr == NULL) {     /* Insert tail elem */
      prev = list->tail;
      list->tail = new;
      new->next = NULL;
      new->prev = prev;
      if (prev == NULL)                         /* Old tail is NULL? */
         list->head = new;
      else
         prev->next = new;

   } else {                                  /* Insert non-tail elem */
      next = curr->next;
      next->prev = new;
      new->next = next;
      curr->next = new;
      new->prev = curr;
   }

   return new;
}

/*-------------------------------------------------------------------*/
/* list_delete                                                       */
/*-------------------------------------------------------------------*/

void *list_delete(LIST *list, void *elem)
{
   LIST_ELEM *prev, *curr, *next;

   curr = (LIST_ELEM *)elem;
   prev = curr->prev;
   next = curr->next;
   free(curr);
   list->count--;

   if (prev == NULL && next == NULL) {         /* Last elem in list? */
      list->tail = NULL;
      list->head = NULL;
       return NULL;
   }

   if (prev == NULL) {                              /* Elem is head? */
      list->head = next;
      next->prev = NULL;
      return next;
   }

   if (next == NULL) {                              /* Elem is tail? */
      list->tail = prev;
      prev->next = NULL;
      return NULL;
   }

   prev->next = next;                  /* Elem is non-head, non-tail */
   next->prev = prev;
   return next;
}
