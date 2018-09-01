#include "queue.h"

#include <stdlib.h>
#include <stdio.h>


void queue_append (queue_t **queue, queue_t *elem) {
	if (queue[0] == NULL){
     printf ("\nFila criada!\n");
     queue[0]=elem;
     elem->prev=elem;
     elem->next = elem;
   }
  else if (elem == NULL)
	   printf ("\nElemento nao existe!\n");
  else if (elem->prev != NULL || elem->next!=NULL)
	   printf ("\nElemento ligado a outra fila!\n");
  else{
     queue_t *ultimo = (queue[0])->prev;
     ultimo->next = elem;
     elem->prev= ultimo;
     elem->next=(queue[0]);
     (queue[0])->prev=elem;
  }
}

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
   if (queue == NULL){
  	 printf ("Fila nao existe!");
  	  return NULL;
   }
   else if (queue[0]==NULL){
     printf ("Fila vazia");
     return NULL;
   }
   else if (elem == 0){
	    printf ("Elemento nao existe!");
      return NULL;
   }
   else {
     int tam = queue_size (queue[0]);
		 if (tam==1 && queue[0]==elem){
				 queue_t *at = queue[0];
				 at->next=NULL;
				 at->prev=NULL;
				 queue[0]=NULL;
				 return at;
		 }
		 if (tam==1 && queue[0]!=elem){
			 		printf ("Elemento nao estava na fila");
					return NULL;
		 }
		 int i;
     for (i=0; i<tam; i++){
			 		if(queue[i] == elem && i==0){
							queue_t *at = queue[i];
							(queue[i])->prev->next=queue[i]->next;
							(queue[i])->next->prev=queue[i]->prev;
							queue[i]=(queue[i])->next;
							at->next=NULL;
							at->prev=NULL;
							return at;
					}
					else if(queue[i] == elem && i!=0){
						queue_t *at = queue[i];
						(queue[i])->prev->next=queue[i]->next;
						(queue[i])->next->prev=queue[i]->prev;
						at->next=NULL;
						at->prev=NULL;
						return at;
    	    }

    	}
    	printf ("\nElemento não pertencia a fila!\n");
	    return NULL;

    }
}

int queue_size (queue_t *queue) {
  if (queue==NULL){
	   printf ("\nFila vazia!\n");
     return 0;
  }
  int i;
  queue_t *elem = queue->next;
  for (i=1; elem!=queue ; i++)
    elem = elem->next ;
  return i;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*)){
  if (queue==NULL)
		printf ("[]");
  else{
		queue_t *elem=queue;
		print_elem (elem);
		printf (" ");
		for (elem=elem->next; elem!=queue; elem=elem->next){
			print_elem (elem);
			printf (" ");
		}
	}
	printf ("\n");

}
