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
    //PRIORIDADE(ENVELHECIMENTO)!
    if (userTasks==1){ //so a main
        return fila0;
        #ifdef DEBUG
            printf ("scheduler retornou a main\n") ;
        #endif
    }
    int i;
    task_t *elem=fila0->next;
    aux=fila0;
    t_main.aging=0;
    int pri_aux = aux->prio_static-aux->aging;
    int pri_elem;

    for (i=0; i<userTasks-1; i++){
        pri_elem=elem->prio_static-elem->aging;
        if ((pri_elem<=pri_aux) && (elem!=&t_main)){
            aux->aging=(aux->aging)+1;
            aux=elem;
            pri_aux = aux->prio_static-aux->aging;
        }
        else if ((pri_elem>pri_aux) && (elem!=&t_main)){
            elem->aging=(elem->aging)+1;
        }

        elem=elem->next;
    }
    #ifdef DEBUG
        printf ("scheduler retornou a task %d com a prioridade %d\n", aux->tid, pri_aux) ;
    #endif
    aux->aging=0;
    return aux;
}

void dispatcher_body (){
    #ifdef DEBUG
        printf ("dispatcher_body chamado\n") ;
    #endif
    //printf ("dispatcher_body: userTasks %d\n", userTasks);
    while (userTasks){
        //printf ("\nvai chamar scheduler\n");
        next = scheduler();
        //printf ("\nsaiu do scheduler \n");
        if (next)
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
    t_main.prio_static=20;
    t_main.aging = 0;
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
    task->prio_static=0;
    task->aging=0;
    makecontext (&(task->context), (void*)(*start_func), 1, arg);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    userTasks++;
    #ifdef DEBUG
        printf ("task_create: criou tarefa %d \n", task->tid);
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

void task_setprio (task_t *task, int prio) {
  if (task){
      task->prio_static = prio;
      #ifdef DEBUG
          printf ("task_setprio1: setou a prio_static da task %d para %d\n", task->tid, task->prio_static) ;
      #endif
  }
  else{
      atual->prio_static = prio;
      #ifdef DEBUG
      printf ("task_setprio: setou a prio_static da task %d para %d\n", atual->tid, task->prio_static) ;
      #endif
  }

}

int task_getprio (task_t *task) {
    if (task){
        return task->prio_static;
        #ifdef DEBUG
            printf ("task_getprio: retornou a prio_static da task %d\n", task->tid) ;
        #endif
    }
    else{
        return atual->prio_static;
        #ifdef DEBUG
            printf ("task_getprio: retornou a prio_static da task %d\n", atual->tid) ;
        #endif
    }

}
