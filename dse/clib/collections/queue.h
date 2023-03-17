/* Copyright (C) 2020 Tyler Barrus */
#ifndef DSE_CLIB_COLLECTIONS_QUEUE_H_
#define DSE_CLIB_COLLECTIONS_QUEUE_H_

/*******************************************************************************
***
***  Author: Tyler Barrus
***  email:  barrust@gmail.com
***
***  Version: 0.1.0
***  Purpose: Generic queue (first in first out [FIFO]) implementation
***
***  License: MIT 2020
***
***  URL: https://github.com/barrust/c-utils
***
***  Usage:
***
***
*******************************************************************************/

#include <stdbool.h>
#include <dse/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __doubly_linked_list  q_list;
typedef struct __doubly_linked_list* queue_list_t;


#define QUEUE_SUCCESS 0
#define QUEUE_FAILURE -1


typedef struct __dll_node {
    void*              data;
    struct __dll_node* next;
    struct __dll_node* prev;
} queue_node;


/*  Initialize the doubly linked list
    Returns:
        NULL        - If error allocating the memory
        dllist_t
*/
DLL_PUBLIC queue_list_t q_init(void);

/*  Free the data from the doubly linked list;
    NOTE: does not free the data element */
DLL_PUBLIC void q_free(queue_list_t q);

/*  Free the data from the doubly linked list;
    NOTE: to free the data, set free_data to true */
DLL_PUBLIC void q_free_alt(queue_list_t q, bool free_data);

/*  Returns the number of nodes in the doubly linked list */
DLL_PUBLIC size_t q_num_elements(queue_list_t q);

/*  Return the head node from the doubly linked list */
DLL_PUBLIC queue_node* q_first_node(queue_list_t q);

/*  Return the tail node from the doubly linked list */
DLL_PUBLIC queue_node* q_last_node(queue_list_t q);


/*  Insert a new node into the queue */
DLL_PUBLIC int q_push(queue_list_t q, void* data);

/*  Pop the node from the front of the queue */
DLL_PUBLIC void* q_pop(queue_list_t q);
DLL_PUBLIC void  q_pop_alt(queue_list_t q, bool free_data);

/*  Traverse the list easily using the following macros */
#define q_traverse(q, node)                                                    \
    for (node = q_first_node(q); node != NULL; node = node->next)
#define q_reverse_traverse(q, node)                                            \
    for (node = q_last_node(q); node != NULL; node = node->prev)

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DSE_CLIB_COLLECTIONS_QUEUE_H_
