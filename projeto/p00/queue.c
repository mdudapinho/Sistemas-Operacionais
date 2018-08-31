#include "queue.h"

#include <stdlib.h>
#include <stdio.h>
//#include <iostream>

/*queue_t * creatQueue_t()
{
   struct queue_t *prev = (queue_t *)malloc(sizeof(queue_t));  // aponta para o elemento anterior na fila
   struct queue_t *next = (queue_t *)malloc(sizeof(queue_t));  // aponta para o elemento seguinte na fila

} */


void queue_append (queue_t **queue, queue_t *elem) {
	//std::cout<< "queue:\t"<< queue << "queue[0]:\t"<< queue[0]<<std::endl;
  printf ("\nEntrou no queue_append!\n");

  if (queue[0] == NULL)
	   printf ("\nFila nao existe!\n");
  else if (elem == NULL)
	   printf ("\nElemento nao existe!\n");
  else if (elem->prev != NULL || elem->next!=NULL)
	   printf ("\nElemento ligado a outra fila!\n");
  else{
     printf ("\nElemento vai ser colocado !\n");

     queue_t *ultimo = (queue[0])->prev;
     ultimo->next = elem;
     elem->prev= ultimo;
     elem->next=(*queue);
     (queue[0])->prev=elem;
  }
  printf ("\nSaiu do queue append\n");
}

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
  printf ("\nEntrou no queue_remove!\n");
/*
  if (queue == NULL){
	printf ("Fila nao existe!");
	return NULL;
   }
   else if (elem == 0){
	printf ("Elemento nao existe!");
        return NULL;
   }
   else {
	queue_t *aux;
	int tam = queue_size (queue[0]);
	int i;
	for (i=0; i<tam; i++){
	    if(queue[i] == elem){
		aux = queue[i];
		(queue[i]->prev)->next=queue[i]->next;
		(queue[i]->next)->prev=queue[i]->prev;
		return aux;
	    }
	}*/
	printf ("\nElemento não pertenci a fila!\n");
	return NULL;


}

int queue_size (queue_t *queue) {
  printf ("\nEntrou no queue_size!\n");

  if (queue==NULL){
	   printf ("\nFila vazia!\n");
     printf ("\nretornou queue_size\n");

	   return 0;
  }
  printf ("\nfila nao vazia \n");
  int i, flag=0;
  queue_t *elem = queue;
  for (i==0; elem!=queue && flag; i++){
	   elem = elem->next ;
     flag=1;
   }
   printf ("\nretornou queue_size\n");
  //return i;
  return i;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*)){
  printf ("\nEntrou no queue_print!\n");
/*
    if (!queue)
	printf ("Fila vazia");

    else{
        int i;
        queue_t *a_elem;
        for (a_elem=queue; a_elem->next==NULL; a_elem=a_elem->next)
	    print_elem (a_elem);
    }*/
    printf ("\nSaiu do queue_print!\n");

}
