#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768
#define N 100
//#define DEBUG 1
// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

task_t t_main, dispatcher, *fila0= NULL,*antigo=NULL,*atual, *next, *fila_espera=NULL, *aux;
int cont_id=1;
int userTasks=0;
int tick=20;
int ticks=0;

void task_yield (){

    #ifdef DEBUG
        printf ("task_yield: vai sair da tarefa %d e ir para o dispatcher\n", atual->tid) ;
    #endif
    task_switch (&dispatcher);
}

void task_yield_temp (){
    ticks++;
    atual->ntick+=1;
    if(tick>0){
      #ifdef DEBUG
          printf ("task_yield_temp: tarefa %d diminuiu o quantuum\n", atual->tid) ;
      #endif
        tick--;
    }
    else{
        tick=19;
        #ifdef DEBUG
            printf ("task_yield_temp: tarefa %d terminou o quantuum\n", atual->tid) ;
        #endif
        task_yield();
    }
}

task_t* scheduler(){
    //FIFO
    /*if (fila0==&(t_main) && userTasks==1){
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
    }*/

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
    while (userTasks && next!=&t_main){
        //printf ("\nvai chamar scheduler\n");
        next = scheduler();
        //printf ("\nsaiu do scheduler \n");
        if (next && next!=&t_main){
            if (setitimer (ITIMER_REAL, &timer, 0) < 0)
            {
              perror ("Erro em setitimer: ") ;
              exit (1) ;
            }
            task_switch(next);}
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
    t_main.prio_static=20;
    t_main.aging = 0;
    t_main.ntick=0;    //numero de ticks usados pela tarefa
    t_main.tick_total=ticks;   //numero de ticks total do processador
    t_main.ativacoes=1;
    atual=&t_main;    //queue_append ((queue_t **) &fila0, (queue_t*) atual);
    setvbuf (stdout, 0, _IONBF, 0) ;
    action.sa_handler = task_yield_temp ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
      perror ("Erro em sigaction: ") ;
      exit (1) ;
    }
   // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
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
    task->ntick=0;    //numero de ticks usados pela tarefa
    task->tick_total=ticks;   //numero de ticks total do processador
    task->ativacoes=0;
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
    if (antigo->status==0){   //entra quando a tarefa for voltar para a fila
        queue_append((queue_t**)&fila0,(queue_t*)antigo);
        userTasks++;
        #ifdef DEBUG
            printf ("task_switch: colocou a tarefa %d na fila\n", antigo->tid) ;
        #endif
    }
    else if (antigo->status==1){    //quando a tarefa chegar ao fim
        antigo->tick_total=ticks-antigo->tick_total;
        printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", antigo->tid, antigo->tick_total, antigo->ntick, antigo->ativacoes);

    }
    #ifdef DEBUG
        printf ("task_switch: passou para a tarefa %d\n", atual->tid) ;
    #endif
    atual->ativacoes=atual->ativacoes+1;
    return swapcontext (&(antigo->context), &(atual->context));
}

void task_exit (int exitCode) {
    atual->status=1;
    #ifdef DEBUG
        printf ("task_exit: vai retirar a tarefa %d\n", atual->tid) ;
    #endif
    if(atual->tid == dispatcher.tid){
        task_switch (&t_main);
    }
    else{
        task_switch (&dispatcher);
    }
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

unsigned int systime (){
  #ifdef DEBUG
      printf ("systime: retornou valor da task %d\n", ticks) ;
  #endif

  return (ticks);

}
