#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#define STACKSIZE 32768
#define N 100
//#define DEBUG
task_t t_main, *fila0= NULL,*antigo=NULL,*atual;
int cont_id=1;
int flag=0;
void pingpong_init () {
    getcontext(&t_main.context);
    t_main.prev = NULL;
    t_main.next = NULL;
    t_main.tid= 0;
    atual=&t_main;    //queue_append ((queue_t **) &fila0, (queue_t*) atual);
    setvbuf (stdout, 0, _IONBF, 0) ;
    #ifdef DEBUG
        printf ("pingpong_init iniciado") ;
    #endif
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
    getcontext(&(task->context));
    char *stack ;
    stack = malloc (STACKSIZE) ;
    if (stack){
       task->context.uc_stack.ss_sp = stack ;
       task->context.uc_stack.ss_size = STACKSIZE;
       task->context.uc_stack.ss_flags = 0;
       task->context.uc_link = 0;
    }
    else{
       perror ("Erro na criao da pilha: ");
       exit (1);
    }
    task->tid=cont_id++;
    task->prev=NULL;
    task->next=NULL;
    makecontext (&(task->context), (void*)(*start_func), 1, arg);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid);
    #endif
    return task->tid;
}
int task_switch (task_t *task) {
    antigo=atual;
    atual=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)task);
    if (!flag){
      queue_append((queue_t**)&fila0,(queue_t*)antigo);

    }
    flag=0;
    #ifdef DEBUG
        printf ("task_switch: passou para a tarefa %d\n", atual->tid) ;
    #endif
    return swapcontext (&(antigo->context), &(atual->context));

}

void task_exit (int exitCode) {
    flag=1;
    #ifdef DEBUG
        printf ("task_exit: vai retirar a tarefa %d\n", atual->tid) ;
    #endif
    task_switch (&t_main);
    //antigo=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)antigo);
    /*antigo=atual;
    atual=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)&t_main);
    swapcontext (&(antigo->context), &(atual->context));*/

}

int task_id () {
    #ifdef DEBUG
        printf ("task_id: retornou valor da task %d\n", atual->tid) ;
    #endif
   return atual->tid;
}
