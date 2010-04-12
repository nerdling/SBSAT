/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2008, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark, service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//-------------Pointer-based Queue Manipulations---------------//

typedef struct cell_queue {
	void *x;
	struct cell_queue *next;
} cell_queue;

typedef struct void_queue {
	cell_queue *head;
	cell_queue *tail;
} void_queue;

//-------------Pointer-based Stack Manipulations---------------//

typedef struct cell_stack {
	void *x;
	struct cell_stack *push;
	struct cell_stack *pop;
} cell_stack;

typedef struct void_stack {
	cell_stack *head;
} void_stack;

//-------------Array-based Stack Manipulations---------------//

typedef struct void_arr_stack {
	uint32_t head;
	uint32_t size;
	void **mem;
} void_arr_stack;

//-------------Pointer-based Queue Manipulations---------------//

void_queue *queue_init();
void queue_free(void_queue *queue);
void enqueue_x(void_queue *queue, void *x);
void *dequeue(void_queue *queue);

//-------------Pointer-based Stack Manipulations---------------//

void_stack *stack_init();
void stack_free(void_stack *stack);
void stack_push(void_stack *stack, void *x);
void *stack_pop(void_stack *stack);

//-------------Array-based Stack Manipulations---------------//

void_arr_stack *arr_stack_init();
void arr_stack_free(void_arr_stack *stack);
void arr_stack_push(void_arr_stack *stack, void *x);
void *arr_stack_pop(void_arr_stack *stack);
