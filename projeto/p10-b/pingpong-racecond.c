#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif


#define N 10//

item_t *fila_itens=NULL;
int contador=0;
semaphore_t *s_vaga, *s_buffer, *s_item;
task_t prod[3], cons[2] ;


void produtor(semaphore_t *s_vaga, semaphore_t *s_buffer, semaphore_t *s_item){
    int item=0;
    while(1){
	task_sleep(1);
	item= random() % 100;
	sem_down(s_vaga);
	sem_down(s_buffer);
	item_t *it = (item_t*)malloc(sizeof(item_t));
	it->n_item = item;
        queue_append((queue_t**)&(fila_itens),(queue_t*)&(it));
	contador+=1;
	printf ("produtor (task %d) colocou %d no buffer\n", task_id(), item);
	sem_up(s_buffer);
	sem_up(s_item);
    }
}
void consumidor(semaphore_t *s_vaga,semaphore_t *s_buffer,semaphore_t *s_item){
    item_t *it_aux=NULL;
    int item=0;
    while(1){
	sem_down(s_item);
	sem_down(s_buffer);
	it_aux = (item_t*)queue_remove((queue_t**)&(fila_itens),(queue_t*)(fila_itens));
    	queue->contador-=1;	
	sem_up(s_buffer);
	sem_up(s_vaga);
	printf ("consumidor (task %d) retirou %d do buffer\n", task_id(), it_aux->n_item);
	task_sleep(1);	
    }
}

int main (int argc, char *argv[])
{
    printf ("Main INICIO\n") ;
    pingpong_init () ;
    sem_create (s_buffer, 5);
    sem_create (s_vaga, N);
    sem_create (s_item, N);
    task_create (&cons[0], consumidor, NULL) ;
    task_create (&cons[1], consumidor, NULL) ;
    task_create (&prod[0], produtor, NULL) ;
    task_create (&prod[1], produtor, NULL) ;
    task_create (&prod[2], produtor, NULL) ; 


    // encerra a thread main 
    printf ("Main FIM\n") ;
    task_exit (0) ;

    exit (0) ;
}

