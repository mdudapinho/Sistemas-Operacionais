#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
/*queue_t * creatQueue_t()
{
   struct queue_t *prev = (queue_t *)malloc(sizeof(queue_t));  // aponta para o elemento anterior na fila
   struct queue_t *next = (queue_t *)malloc(sizeof(queue_t));  // aponta para o elemento seguinte na fila

} */




void queue_append (queue_t **queue, queue_t *elem) {
   if (queue == NULL)
	printf ("Fila nao existe!");
   else if (elem == NULL)
	printf ("Elemento nao existe!");
   else if (elem->prev != NUL || elem->next!=NULL)
	printf ("Elemento ligado a outra fila!");
   else{
	queue_t *a_elem;
	for (a_elem=queue[0]; a_elem->next==NULL; a_elem=a_elem->next)
	    elem = elem->next ;
	a_elem->next = elem;
	elem->prev = a_elem;
   }

}

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
   if (queue == NULL){
	printf ("Fila nao existe!");
	return NULL;
   }
   else if (n = 0){
	printf ("Elemento nao existe!");
        return NULL;
   }
   else {
	queue_t *aux;
	int tam = queue_size (queue[0])
	int i;
	for (i=0; i<tam; i++){
	    if(queue[i] == elem){
		aux = queue[i];
		(queue[i]->prev)->next=queue[i]->next;
		(queue[i]->next)->prev=queue[i]->prev;
		return aux;
	    }
	}
	printf ("Elemento não pertenci a fila!");
	return NULL;

   }
}

int queue_size (queue_t *queue) {
    if (!queue){
	printf ("Fila vazia");
	return 0;
    }
    int i;
    queue_t *elem = queue;
    for (i==0; elem!=NULL; i++)
	elem = elem->next ;
    return i;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*){
    if (!queue)
	printf ("Fila vazia");

    else{
        int i;
        queue_t *a_elem;
        for (a_elem=queue[0]; a_elem->next==NULL; a_elem=a_elem->next)
	    print_elem (a_elem);
    }
}
