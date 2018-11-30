#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif




item_t *fila_itens=NULL;
int contador=0;
semaphore_t s_vaga, s_buffer, s_item;
task_t prod[3], cons[2] ;


void produtor(){
    int item=0;
    while(1){
	task_sleep(1);
	//printf ("task %d voltou dps de dormir\n", task_id());
	item= random() % 10;
	//printf ("item=%d", item);
	sem_down(&s_vaga);
	sem_down(&s_buffer);
	item_t *it = (item_t*)malloc(sizeof(item_t));
	it->next =NULL;
	it->prev=NULL;
	it->n_item = item;
        queue_append((queue_t**)&(fila_itens),(queue_t*)(it));
	contador+=1;
	printf ("produtor %d colocou %d no buffer\n", task_id(), item);
	sem_up(&s_buffer);
	sem_up(&s_item);
/*
	if (item == 0){
		printf ("tchau produtor %d!\n",task_id() );
		task_exit (0) ;
	}
*/
    }
}
void consumidor(){
    item_t *it_aux=NULL;
    int item=0;
    while(1){
	sem_down(&s_item);
	sem_down(&s_buffer);
	it_aux = (item_t*)queue_remove((queue_t**)&(fila_itens),(queue_t*)(fila_itens));
    	contador-=1;	
	sem_up(&s_buffer);
	sem_up(&s_vaga);
	printf ("		consumidor %d retirou %d do buffer\n", task_id(), it_aux->n_item);
	task_sleep(1);
/*
	if (it_aux->n_item == 0){
		printf ("tchau consumidor %d!\n",task_id() );
		task_exit (0) ;
	}
*/	
    }
}

int main (int argc, char *argv[])
{
    printf ("Main INICIO\n") ;
    pingpong_init () ;
    printf ("Main criando semaforos\n") ;
    sem_create (&s_buffer, 5);
    sem_create (&s_vaga, 1);
    sem_create (&s_item, 0);
    printf ("Main criando tasks\n") ;
    task_create (&cons[0], consumidor, NULL) ;
    task_create (&cons[1], consumidor, NULL) ;
    task_create (&prod[0], produtor, NULL) ;
    task_create (&prod[1], produtor, NULL) ;
    task_create (&prod[2], produtor, NULL) ; 

    task_join (&cons[0]);
    task_join (&cons[1]);
    task_join (&prod[0]);
    task_join (&prod[1]);
    task_join (&prod[2]);



    printf ("Main destruindo semaforos\n") ;
    sem_destroy(&s_buffer);
    sem_destroy(&s_vaga);
    sem_destroy(&s_item);
    // encerra a thread main 
    printf ("Main FIM\n") ;
    task_exit (0) ;

    exit (0) ;
}

