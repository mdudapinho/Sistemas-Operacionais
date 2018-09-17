#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#define STACKSIZE 32768
#define N 100

task_t t_main, dispatcher, *fila0= NULL,*antigo=NULL,*atual, *next, *fila_espera=NULL, *aux;
int cont_id=1;
int userTasks=0;

task_t* scheduler(){
    //FIFO
    if (fila0==&(t_main) && userTasks==1){
        #ifdef DEBUG
            printf ("scheduler: passou para a task %d\n", fila0->tid) ;
        #endif
        return fila0;
    }
    else if(fila0==&(t_main) && userTasks>1){
        #ifdef DEBUG
            printf ("scheduler: passou para a task %d\n", fila0->next->tid) ;
        #endif
        return fila0->next;
    }
    else {
        #ifdef DEBUG
            printf ("scheduler: passou para a task %d\n", fila0->next->tid) ;
        #endif
        return fila0;
    }

}

void dispatcher_body (){
    #ifdef DEBUG
        printf ("dispatcher_body chamado\n") ;
    #endif
    while (userTasks && next!=&t_main){
        //printf ("\nvai chamar scheduler\n");
        next = scheduler();
        //printf ("\nsaiu do scheduler \n");
        if (next && next!=&t_main)
            task_switch(next);
    }
    #ifdef DEBUG
        printf ("saiu do dispatcher_body\n") ;
    #endif
    task_exit(0);
}

void pingpong_init () {
    getcontext(&t_main.context);
    t_main.prev = NULL;
    t_main.next = NULL;
    t_main.tid= 0;
    t_main.status=0;
    atual=&t_main;    //queue_append ((queue_t **) &fila0, (queue_t*) atual);
    setvbuf (stdout, 0, _IONBF, 0) ;
    task_create(&dispatcher,dispatcher_body,NULL);
    #ifdef DEBUG
        printf ("pingpong_init iniciado\n") ;
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
    task->status=0;
    makecontext (&(task->context), (void*)(*start_func), 1, arg);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    userTasks++;
    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid);
    #endif
    return task->tid;
}

int task_switch (task_t *task) {
    antigo=atual;
    atual=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)task);
    userTasks--;
    if (antigo->status==0){
        queue_append((queue_t**)&fila0,(queue_t*)antigo);
        userTasks++;
        #ifdef DEBUG
            printf ("task_switch: colocou a tarefa %d na fila\n", antigo->tid) ;
        #endif
    }
    #ifdef DEBUG
        printf ("task_switch: passou para a tarefa %d\n", atual->tid) ;
    #endif
    return swapcontext (&(antigo->context), &(atual->context));
}

void task_exit (int exitCode) {
    atual->status=1;
    #ifdef DEBUG
        printf ("task_exit: vai retirar a tarefa %d\n", atual->tid) ;
    #endif
    if(atual->tid == dispatcher.tid)
        task_switch (&t_main);
    else
        task_switch (&dispatcher);
}

int task_id () {
    #ifdef DEBUG
        printf ("task_id: retornou valor da task %d\n", atual->tid) ;
    #endif
   return atual->tid;
}

void task_suspend (task_t *task, task_t **queue) {
    if (!(queue==NULL)){
        fila_espera=*queue;
        if (task == NULL){
            aux = atual;
            aux->status=2;
            queue_append((queue_t**)queue,(queue_t*)aux);
            userTasks--;
            #ifdef DEBUG
                printf ("task_suspend: suspendendo tarefa %d \n", aux->tid) ;
            #endif
            task_switch(&dispatcher);
        }
        else {
            aux= (task_t*)queue_remove((queue_t**)&fila0,(queue_t*)task);
            aux->status=2;
            queue_append((queue_t**)queue,(queue_t*)aux);
            userTasks--;
            #ifdef DEBUG
                printf ("task_suspend: suspendendo tarefa %d \n", aux->tid) ;
            #endif
        }

    }
}

void task_resume (task_t *task){
    aux= (task_t*)queue_remove((queue_t**)&fila_espera,(queue_t*)task);
    if (aux){
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks++;
    }
    #ifdef DEBUG
        printf ("task_resume: devolvendo tarefa %d a fila de prontas\n", aux->tid) ;
    #endif


}

void task_yield (){
    #ifdef DEBUG
        printf ("task_yield: vai sair da tarefa %d e ir para o dispatcher\n", atual->tid) ;
    #endif
    task_switch (&dispatcher);
}
