#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#define STACKSIZE 32768
#define N 100
task_t *atual=NULL, *t_main=NULL, *fila0=NULL;
task_t aux[N];
int contador=0;

void pingpong_init () {
    ucontext_t ContextMain;
    getcontext(&ContextMain);
    aux[contador].tid = 0;
    aux[contador].prev=NULL;
    aux[contador].next=NULL;
    aux[contador].context = ContextMain;
    t_main = &aux[contador];
    queue_append ((queue_t **) &fila0, (queue_t*) t_main);
    atual=t_main;
    contador++;
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
    ucontext_t contexto;
    getcontext(&contexto);

    char *stack ;
    stack = malloc (STACKSIZE) ;
    if (stack)
    {
       contexto.uc_stack.ss_sp = stack ;
       contexto.uc_stack.ss_size = STACKSIZE;
       contexto.uc_stack.ss_flags = 0;
       contexto.uc_link = 0;
    }
    else
    {
       perror ("Erro na cria��o da pilha: ");
       exit (1);
    }

    makecontext (&contexto, (void*)(*start_func), 1, arg);
    aux[contador].context = contexto;
    aux[contador].tid = contador;
    aux[contador].prev=NULL;
    aux[contador].next=NULL;
    task= &aux[contador];
    contador++;
    queue_append ((queue_t **) &fila0, (queue_t*) task);

    if ((fila0->prev) ==  task)
        return -1;    //se nao tiver colocado a task na fila retorna um numero negativo.
    return task->tid;
}

void task_exit (int exitCode) {
    task_switch (t_main);
    atual=t_main;

}

int task_switch (task_t *task) {
    swapcontext (&(atual->context), &(task->context));
    atual->context = task->context;
    if (atual != task)
      return -1;
    return 0;

}

int task_id () {
    return atual->tid;

}
