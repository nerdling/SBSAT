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

#include "queue.h"

#define QUEUE_INCREASE_SIZE 1000
#define STACK_INCREASE_SIZE 1000

//-------------Pointer-based Queue Manipulations---------------//

void increase_queue(void_queue *queue) {
	cell_queue *tmp = queue->tail;
	for(int i = 0; i < QUEUE_INCREASE_SIZE; i++) {
		tmp->next = (cell_queue *)malloc(sizeof(cell_queue));
		tmp = tmp->next;
	}
	tmp->next = queue->head;
}

void_queue *queue_init() {
	void_queue *queue = (void_queue *)malloc(sizeof(void_queue));
	queue->head = (cell_queue *)malloc(sizeof(cell_queue));
	cell_queue *tmp = queue->head;
	for(int i = 0; i < QUEUE_INCREASE_SIZE; i++) {
		tmp->next = (cell_queue *)malloc(sizeof(cell_queue));
		tmp = tmp->next;
	}
	tmp->next = queue->head;
	queue->tail = queue->head;
	return queue;
}

void queue_free(void_queue *queue) {
	assert(queue != NULL);
	if(queue->head!=queue->tail)
	  fprintf(stderr, "Warning: queue being free'd while objects are still in the queue.\n");
	
	cell_queue *tmp = queue->head;
	queue->head = queue->head->next;
	tmp->next = NULL;
	
	while(queue->head != NULL) {
		tmp = queue->head;
		queue->head = queue->head->next;
		free(tmp);
	}
	free(queue);
}

void enqueue_x(void_queue *queue, void *x) {
	assert(x != NULL);
	if(queue->tail->next == queue->head)
	  increase_queue(queue);
	queue->tail = queue->tail->next;
	queue->tail->x = x;
}

void *dequeue(void_queue *queue) {
	if(queue->head == queue->tail) return NULL;
	queue->head = queue->head->next;
	void *x = queue->head->x;
	return x;	
}

//-------------Pointer-based Stack Manipulations---------------//

void increase_stack(void_stack *stack) {
	cell_stack *tmp = stack->head;
	for(int i = 0; i < STACK_INCREASE_SIZE; i++) {
		tmp->push = (cell_stack *)malloc(sizeof(cell_stack));
		tmp->push->pop = tmp;
		tmp = tmp->push;
	}
	tmp->push = NULL;
}

void_stack *stack_init() {
	void_stack *stack = (void_stack *)malloc(sizeof(void_stack));
	stack->head = (cell_stack *)malloc(sizeof(cell_stack));
	increase_stack(stack);
	return stack;
}

void stack_free(void_stack *stack) {
	assert(stack != NULL);
	if(stack->head->pop!=NULL)
	  fprintf(stderr, "Warning: stack being free'd while objects are still in the stack.\n");
	
	while(stack->head != NULL) {
		cell_stack *tmp = stack->head;
		stack->head = stack->head->push;
		free(tmp);
	}
	free(stack);
}

void stack_push(void_stack *stack, void *x) {
	assert(x != NULL);
	if(stack->head->push==NULL)
	  increase_stack(stack);
	stack->head = stack->head->push;
	stack->head->x = x;
}

void *stack_pop(void_stack *stack) {
	if(stack->head->pop == NULL) return NULL;
	void *x = stack->head->x;
	stack->head = stack->head->pop;
	return x;	
}

//-------------Array-based Stack Manipulations---------------//

void increase_arr_stack(void_arr_stack *stack) {
	stack->mem = (void **)realloc(stack->mem, (stack->size+STACK_INCREASE_SIZE) * sizeof(void *));
	stack->size += STACK_INCREASE_SIZE;
}

void_arr_stack *arr_stack_init() {
	void_arr_stack *stack = (void_arr_stack *)malloc(sizeof(void_arr_stack));
	stack->mem = (void **)malloc(STACK_INCREASE_SIZE * sizeof(void *));
	stack->head = 0;
	stack->size = STACK_INCREASE_SIZE;
	return stack;
}

void arr_stack_free(void_arr_stack *stack) {
	assert(stack != NULL);
	if(stack->head!=0)
	  fprintf(stderr, "Warning: stack being free'd while objects are still in the stack.\n");

	free(stack->mem);
	free(stack);
}

void arr_stack_push(void_arr_stack *stack, void *x) {
	if(stack->head>=stack->size-2)
	  increase_arr_stack(stack);
	stack->mem[++stack->head] = x;
}

void *arr_stack_pop(void_arr_stack *stack) {
	if(stack->head == 0) return NULL;
	void *x = stack->mem[stack->head];
	stack->head--;
	return x;
}

