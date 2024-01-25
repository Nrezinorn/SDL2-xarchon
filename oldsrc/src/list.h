/*-------------------------------------------------------------------*/
/* Double-linked List                                                */
/*-------------------------------------------------------------------*/

#ifndef __MY_LIST_H
#define __MY_LIST_H

/*-------------------------------------------------------------------*/
/* Structures                                                        */
/*-------------------------------------------------------------------*/

typedef struct LIST_ELEM {
   struct LIST_ELEM *next;
   struct LIST_ELEM *prev;
} LIST_ELEM;

typedef struct {
   LIST_ELEM *head;
   LIST_ELEM *tail;
   int count;
} LIST;

/*-------------------------------------------------------------------*/
/* Macros                                                            */
/*-------------------------------------------------------------------*/

/* Return head elem of list LIST */
#define list_head(list) ((void *)(((LIST *)(list))->head))

/* Return tail elem of list LIST */
#define list_tail(list) ((void *)((((LIST *)(list))->tail))

/* Return count of elems in list LIST */
#define list_count(list) (((LIST *)(list))->count)

/* Return elem next to elem ELEM */
#define list_next(elem) ((void *)(((LIST_ELEM *)(elem))->next))

/* Return elem previous to elem ELEM */
#define list_prev(elem) ((void *)(((LIST_ELEM *)(elem))->prev))

/*-------------------------------------------------------------------*/
/* Functions                                                         */
/*-------------------------------------------------------------------*/

/* Initialize list pointed to by LIST */
void list_create(LIST *list);

/* Insert new elem before that pointed to by ELEM, return new elem */
void *list_insert_before(LIST *list, void *elem, int elemlen);

/* Insert new elem after that pointed to by ELEM, return new elem */
void *list_insert_after(LIST *list, void *elem, int elemlen);

/* Delete elem pointed to by ELEM, return its next elem */
void *list_delete(LIST *list, void *elem);

#endif /* __MY_LIST_H */
