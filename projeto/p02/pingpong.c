#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#define STACKSIZE 32768
#define N 100
task_t t_main, *fila0= NULL,*antigo=NULL;
int cont_id=1;

void pingpong_init () {
    getcontext(&t_main.context);
    t_main.prev = NULL;
    t_main.tid= 0;
    t_main.next = NULL;
    queue_append ((queue_t **) &fila0, (queue_t*) &t_main);
    setvbuf (stdout, 0, _IONBF, 0) ;
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
    return task->tid;
}

int task_switch (task_t *task) {
    antigo=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)fila0->prev);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    swapcontext (&(antigo->context), &(task->context));
    return 0;
}

void task_exit (int exitCode) {
    task_switch (&t_main);
}

int task_id () {
   return fila0->prev->tid;
}
