#include "queue.h"

#include <stdlib.h>
#include <stdio.h>


void queue_append (queue_t **queue, queue_t *elem) {
	if(queue == NULL)
		printf("fila nao criada\n");
	else if (elem == NULL)
			printf ("Elemento nao existe!\n");
	else if (elem->prev != NULL || elem->next!=NULL)
			printf ("\nElemento ligado a outra fila!\n");
	else if (*queue == NULL){
			printf ("\nFila criada!\n");
			*queue=elem;
			elem->prev=elem;
			elem->next = elem;
	 }
	else {
			elem->next=*queue;
			elem->prev= elem->next->prev;
			elem->prev->next = elem;
			elem->next->prev = elem;
	}
}

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    if (queue == NULL)
  	 	printf ("Fila nao existe!");
    else if (*queue==NULL)
    	printf ("Fila vazia");
    else {

    	queue_t *rem ;
    	for (rem = *queue;rem!=elem && rem->next!=*queue ;rem=rem->next);
    	if (rem == elem){
    		if(rem==*queue){
			//printf ("tas.kjdnçiawudbnçIQWUDBa na queue\n");

    			if (rem->next==rem)
    				*queue=NULL;

    			else{
				*queue=rem->next;
				//printf("\naqui mesmo\n");
			}
    		}
    		rem->next->prev=rem->prev;
    		rem->prev->next=rem->next;
    		rem->next=NULL;
    		rem->prev=NULL;
    		return rem;
    	}
    	printf("elemento nao existe!\n");
    }
    return NULL;
}

int queue_size (queue_t *queue) {
	if (queue!=NULL){
		int i;
		queue_t *elem = queue->next;
		for (i=1; elem!=queue ; i++){
			elem = elem->next ;
		}
		return i;
	}
	return 0;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*)){
  	queue_t *elem = queue;
  	while(queue != NULL && elem->next!=queue){
  		print_elem (elem);
		printf (" ");
		elem=elem->next;
	}
  	print_elem(queue);
	printf ("\n");

}
