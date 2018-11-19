
#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768
#define N 100
//
#define DEBUG 1
struct sigaction action ;
struct itimerval timer;

task_t t_main, dispatcher;
task_t *fila0= NULL,*antigo=NULL,*atual, *next, *fila_espera=NULL, *aux;
task_t *adormecidas=NULL, *suspensas_mensagens=NULL; //Suspensas->por causa da fila de mensagens

int cont_id=1;
int userTasks=0;
int tasksSemaforo =0;
int tick=19;
int ticks=0; //contador geral de ticks
int valor_task_exit=0;
int tasksAdormecidas=0;
int tasksBarreira=0;
int tasksMensagens=0;

typedef struct mensagem_t{
    struct mensagem_t *prev;
    struct mensagem_t *next;
    void *msg;
} mensagem_t ;

mensagem_t *auxiliar;

void task_yield (){
    #ifdef DEBUG
        //printf ("task_yield: vai sair da tarefa %d e ir para o dispatcher\n", atual->tid) ;
    #endif
    if (atual->tid !=dispatcher.tid){
        task_switch (&dispatcher);
    }
}

void acorda_adormecidas(){
  /*Percorre a fila de tarefas adormecidas, diminui o tempo de todas e verifica quem precisa ser acordada*/
    if (tasksAdormecidas){
        task_t* elem=adormecidas;
        int inicio, acorda=0;
        for (inicio=0; inicio<tasksAdormecidas; inicio++){
            elem->adormecida--;
            elem=elem->next;
            if (elem->adormecida<=0){
                //printf ("tem que acordar a task %d\n", elem->tid);
                acorda++;
            }
        }
        //printf ("tem que acordar %d tasks\n", acorda);
        elem=adormecidas;
        task_t* anterior;
        while(acorda){
          if (elem->adormecida<=0){
              anterior=elem->prev;
              aux=(task_t*)queue_remove((queue_t**)&adormecidas,(queue_t*)elem);
              tasksAdormecidas--;
              queue_append((queue_t**)&fila0,(queue_t*)aux);
              userTasks++;
              aux->status=0;
              acorda--;
              elem=anterior;
              #ifdef DEBUG
                  printf ("acorda_adormecidas: acordando a tarefa %d\n", aux->tid) ;
              #endif
          }
          elem=elem->next;
        }
    }
}

void task_yield_temp (){
    ticks++;
    atual->ntick+=1;
    if (!(ticks%1000))
	printf(" ");
	//printf ("\t %d ms\n", ticks);

    acorda_adormecidas();
    if(tick>0){
	tick--;
    }
    else{
        tick=19;
        task_yield();
    }
}

task_t* scheduler(){
    //PRIORIDADE(ENVELHECIMENTO)!
    if (userTasks==0){
        return NULL;
    }
    int i;
    task_t *elem=fila0->next;
    aux=fila0;
    int pri_aux = aux->prio_static-aux->aging;
    int pri_elem;
    for (i=0; i<userTasks; i++){
        pri_elem=elem->prio_static-elem->aging;
        if ((pri_elem<=pri_aux) ){
            aux->aging=(aux->aging)+1;
            aux=elem;
            pri_aux = aux->prio_static-aux->aging;
        }
        else if ((pri_elem>pri_aux) ){
            elem->aging=(elem->aging)+1;
        }

        elem=elem->next;
    }
    #ifdef DEBUG
        //printf ("scheduler retornou a task %d com a prioridade %d\n", aux->tid, pri_aux) ;
    #endif
    aux->aging=0;
    return aux;
}

void dispatcher_body (){
    #ifdef DEBUG
        printf ("dispatcher_body chamado\n") ;
    #endif
    while (userTasks || tasksAdormecidas|| tasksSemaforo || tasksBarreira){
        next = scheduler();

        if (next){
	     //printf ("dispatcher_body\t userTasks %d||tasksAdormecidas %d||tasksSemaforo %d \n",userTasks,tasksAdormecidas,tasksSemaforo);
            if (setitimer (ITIMER_REAL, &timer, 0) < 0)
            {
              perror ("Erro em setitimer: ") ;
              exit (1) ;
            }
            task_switch(next);
	    //printf("voltou no dispatcher dps de trocar com a proxima tarefa\n");
        }
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
    t_main.prio_static=0;
    t_main.aging = 0;
    t_main.ntick=0;    //numero de ticks usados pela tarefa
    t_main.tick_total=ticks;   //numero de ticks total do processador
    t_main.ativacoes=1;
    t_main.parent=NULL;
    t_main.exit_parent=0;
    t_main.adormecida=0;
    t_main.cod_erro_sem=0;
    t_main.cod_erro_bar=0;
    t_main.cod_erro_fila_mensagem=0;
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
        printf ("pingpong_init iniciado: main iniciada com id %d\n", t_main.tid) ;
    #endif
    task_yield();


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
    task->parent=NULL;
    task->exit_parent=0;
    task->adormecida=0;
    task->cod_erro_sem=0;
    task->cod_erro_bar=0;
    task->cod_erro_fila_mensagem=0;
    makecontext (&(task->context), (void*)(*start_func), 1, arg);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    userTasks++;

    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid);
    #endif
    //task_yield();
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

    atual->ativacoes=atual->ativacoes+1;
    #ifdef DEBUG
        printf ("task_switch: passou da tarefa %d para a %d\n", antigo->tid, atual->tid) ;
    #endif
    return swapcontext (&(antigo->context), &(atual->context));
}

void task_exit (int exitCode) {
    //printf ("entrou exit\n");
    atual->status=1;
    valor_task_exit=exitCode;
    #ifdef DEBUG
        printf ("task_exit: vai retirar a tarefa %d\n", atual->tid) ;
    #endif
    fila_espera=(atual->parent);
    while(fila_espera){
        task_resume (atual->parent);
    }
    //printf ("exit:\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo) ;
    if(atual->tid != dispatcher.tid){
        task_switch (&dispatcher);
    }
    else {    //quando o dispatcher chegar ao fim
        userTasks--;
        atual->tick_total=ticks-atual->tick_total;
        printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", atual->tid, atual->tick_total, atual->ntick, atual->ativacoes);
    }

}

int task_id () {
    #ifdef DEBUG
        printf ("task_id: retornou valor da task %d\n", atual->tid) ;
    #endif
   return atual->tid;
}

void task_suspend (task_t *task, task_t **queue) {
    if (task == NULL){
        aux = atual;
        aux->status=2;
        queue_append((queue_t**)queue,(queue_t*)aux);
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
        task_switch(&dispatcher);
        #ifdef DEBUG
            printf ("task_suspend: suspendendo tarefa %d \n", aux->tid) ;
        #endif


    }
}

void task_resume (task_t *task){
    aux= (task_t*)queue_remove((queue_t**)&fila_espera,(queue_t*)task);
    if (aux){
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks++;
        aux->exit_parent=valor_task_exit;
    }
    #ifdef DEBUG
        printf ("task_resume: devolvendo tarefa %d a fila de prontas\nusertask: %d\n", aux->tid, userTasks) ;
    #endif
}

unsigned int systime (){
  #ifdef DEBUG
      printf ("systime: retornou valor da task %d\n", ticks) ;
  #endif

  return (ticks);

}

// a tarefa corrente aguarda o encerramento de outra task
/*A tarefa atual e colocada suspensa na fila da tarefa task->parent ate que a mesma finalize*/
int task_join (task_t *task) {
    if(task!=NULL && task->status!=1 ) {
        #ifdef DEBUG
            printf ("task_join: suspendendo tarefa %d a fila de prontas\n", atual->tid) ;
        #endif
        task_suspend (NULL, &(task->parent));
	printf("saiu do taskjoin\tvalor_task_exit: %d\n", valor_task_exit);
        return valor_task_exit;
    }
    else{
        #ifdef DEBUG
            printf ("task_join: nao achou tarefa\n") ;
        #endif
        return -1;
    }
}

// suspende a tarefa corrente por t segundos
void task_sleep (int t){
    /*passa a tarefa atual para uma fila de tarefas adormecidas e muda o parametro "adormecida" para o tempo em que a tera tera de ficar dormindo*/
    atual->adormecida=1000*t;
    queue_append((queue_t**)&adormecidas,(queue_t*)atual);
    tasksAdormecidas++;
    atual->status=3;
    //printf ("task_sleep: adormecendo a tarefa %d por %d ms \n", atual->tid, atual->adormecida) ;

    #ifdef DEBUG
        printf ("task_sleep: (antes)adormecendo a tarefa %d por %d ms \n", atual->tid, atual->adormecida) ;
    #endif
    task_switch (&dispatcher);
	//printf("eita\n\n");
    #ifdef DEBUG
        printf ("\n\ntask_sleep: (depois)a tarefa %d acordou \n\n", atual->tid) ;
    #endif
}

// semáforos
/*cria um semáforo com valor inicial "value", cada semaforo tem uma variavel status p saber se
o mesmo pode ser usado e um contador para saber quantas tarefas tao na fila desse semaforo*/
int sem_create (semaphore_t *s, int value) {
  //  printf("sem_create\n" );
    if (s->status==1){
      	#ifdef DEBUG
            printf ("sem_create: semaforo ja existe\n") ;
        #endif
    	  return -1;
    }
  //  printf("sem_create\n" );
    s->status=1;
    s->valor = value;
    s->contador=0;
    s->fila = NULL;
    #ifdef DEBUG
        printf ("sem_create: semaforo criado com valor %d\n", value) ;
    #endif
    return 0;
}
// requisita o semáforo
/*olhar no livro, ta mais bem explicado:
Se tiver uma tarefa esperando na fila, a tarefa atual aguarda na fila do semaforo, caso nao tenha,
ela pode continuar a sua execucao*/
int sem_down (semaphore_t *s) {
    if (s->status!=1){
       #ifdef DEBUG
          printf ("sem_down: semaforo inexistente\n") ;
       #endif
	     return -1;
    }
    s->valor--;
    if (s->valor<0){
	      atual->status=4;
        queue_append((queue_t**)&(s->fila),(queue_t*)atual);
        s->contador++;
	      tasksSemaforo++;
        #ifdef DEBUG
            printf ("sem_down: colocando a tarefa %d no semaforo\n", atual->tid) ;
            printf ("sem_down\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo) ;

        #endif
        //printf("sem_down inicio fim 2\n");
	      task_switch (&dispatcher);
    }
    if (atual->cod_erro_sem=1){
        #ifdef DEBUG
           printf ("sem_down: task com codigo de erro\n") ;
        #endif
        return -1;
    }
    return 0;
}
// libera o semáforo
/*Acorda a proxima tarefa que estiver esperando na fila do semaforo-se houver alguma, claro*/
int sem_up (semaphore_t *s) {
    //printf("sem_up inicio\n");
    if (s->status!=1){
	      #ifdef DEBUG
            printf ("sem_down: semaforo inexistente\n") ;
        #endif
	      return -1;
    }
    s->valor++;
    if (s->valor<=0){
      	aux= (task_t*)queue_remove((queue_t**)&(s->fila),(queue_t*)(s->fila));
        s->contador--;
      	tasksSemaforo--;
      	aux->status =0;
      	queue_append((queue_t**)&fila0,(queue_t*)aux);
      	userTasks++;
	        #ifdef DEBUG
            printf ("sem_up: semaforo ativando tarefa %d\n", aux->tid) ;
            printf ("sem_up:\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo) ;
        #endif
    }
    return 0;
}
// destroi o semáforo e as tarefas que estavam na fila do semaforo passam para a fila de prontas
int sem_destroy (semaphore_t *s) {
    if (s->status!=1){
  	  #ifdef DEBUG
          printf ("sem_down: semaforo inexistente\n") ;
      #endif
  	  return -1;
    }
    s->status=0;
    task_t *elem = s->fila;
    while(s->contador>0){
	     #ifdef DEBUG
            printf ("sem_down: semaforo removendo tarefa %d do semaforo e colocando na fila \n", (s->fila)->tid) ;
            printf ("sem_destroy\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo) ;
        #endif

      	aux= (task_t*)queue_remove((queue_t**)&elem,(queue_t*)elem);
        s->contador--;
        aux->status=0;
        aux->cod_erro_sem=1;
      	tasksSemaforo--;
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks++;
    }
    s=NULL;
    return 0;
}

// barreiras
// Inicializa uma barreira
int barrier_create (barrier_t *b, int n) {
    if (b->status==1){
      #ifdef DEBUG
          printf ("barrier_create: problema ao criar barreira - barreira ja existente!\n") ;
      #endif
        return -1;
    }
    b->status=1;
    b->lim=n;
    b->contador=0;
    b->fila=NULL;
    if(b){
        #ifdef DEBUG
            printf ("barrier_create: barreira criada!\n") ;
        #endif
        return 0;
    }
    #ifdef DEBUG
        printf ("barrier_create: problema ao criar barreira - barreira nao criada!\n") ;
    #endif
    return -1;

}

// Chega a uma barreira
/*Se a barreira tiver cheia (com o limite alcancado), ela deve liberar todas as tarefas, ou seja,
retornar elas para a fila de tarefas prontas*/
int barrier_join (barrier_t *b) {
    if (b->status!=1){
        #ifdef DEBUG
            printf ("barrier_join: Barreira nao existe\n") ;
        #endif
        return -1;
    }
    /*Se tiver faltando so uma tarefa, ela nao precisa entrar na fila da barreira porque ela vai ser liberada logo depois*/
    if(b->contador<b->lim-1){
        atual->status=5;
        queue_append((queue_t**)&(b->fila),(queue_t*)atual);
        b->contador++;
        tasksBarreira++;
        #ifdef DEBUG
            printf ("barrier_join: colocando a tarefa %d na barreira com %d tasks\n", atual->tid, b->contador) ;
            printf ("barrier_join\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d || tasksBarreira %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo, tasksBarreira) ;
        #endif
        task_switch (&dispatcher);
        return 0;
    }
    #ifdef DEBUG
        printf ("barrier_join: barreira a tarefa %d acordou as demais e continua \n", atual->tid) ;
    #endif
    task_t *elem = b->fila;
    /*Vai liberando quem estiver na barreira ate nao restar nenhuma task*/
    while(b->contador){
        #ifdef DEBUG
            printf ("barrier_join: barreira removendo tarefa %d e colocando na fila de prontas \n", (b->fila)->tid) ;
            printf ("barrier_join\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d || tasksBarreira %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo, tasksBarreira) ;
        #endif
        aux= (task_t*)queue_remove((queue_t**)&elem,(queue_t*)elem);
        aux->status=0;
        b->contador--;
        tasksBarreira--;
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks++;
    }
}

// Destrói uma barreira
/*Se tiver alguma tarefa na barreira vai liberando, ou seja, passa para a fila de prontas*/
int barrier_destroy (barrier_t *b) {
  if (b->status!=1){
      #ifdef DEBUG
          printf ("barrier_destroy: semaforo inexistente\n") ;
      #endif
      return -1;
  }
  b->status=0;
  task_t *elem = b->fila;
  while(b->contador>0){
     #ifdef DEBUG
          printf ("barrier_destroy: barreira removendo tarefa %d e colocando na fila \n", (b->fila)->tid) ;
          printf ("barrier_join\t userTasks %d || tasksAdormecidas %d || tasksSemaforo %d || tasksBarreira %d \n\n", userTasks, tasksAdormecidas, tasksSemaforo, tasksBarreira) ;
      #endif

      aux= (task_t*)queue_remove((queue_t**)&elem,(queue_t*)elem);
      b->contador--;
      aux->status=0;
      aux->cod_erro_bar=1;
      tasksBarreira--;
      queue_append((queue_t**)&fila0,(queue_t*)aux);
      userTasks++;
  }
  b=NULL;
  return 0;
}

//Fila de mensgaens
int mqueue_create (mqueue_t *queue, int max, int size) {
    #ifdef DEBUG
        printf ("mqueue_create: aqui com a tarefa %d\n", atual->tid) ;
    #endif
    if (queue->status==1){
        #ifdef DEBUG
            printf ("mqueue_create: fila de mensagens ja existe\n") ;
        #endif
      return -1;
    }
    queue->status=1;
    queue->max=max;
    queue->size=size;
    queue->contador=0;
    sem_create (&(queue->sem_rec), 0);
    sem_create (&(queue->sem_sen), max);
    queue->mensagens=NULL;
    #ifdef DEBUG
        printf ("mqueue_create: fila de mensagens criada com tamanho de %d e maximo de %d mensagens \n", queue->size, queue->max) ;
    #endif
    return 0;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) {
    #ifdef DEBUG
        printf ("mqueue_send: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_send: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    int *a =  msg; //endereco da mensagem
    //printf("a: %ld\t %ld\n", a, *a);
    //printf ("sizeof(msg) %ld\t sizeof(queue->size)%ld \n", sizeof(*a), sizeof(queue->size));
    if (sizeof(*a)!=sizeof(queue->size)){
        #ifdef DEBUG
            printf ("mqueue_send: mensagem fora do padrao\ntamanho da mensagem %ld\ttamanho aceito%ld\n", sizeof(*a), sizeof(queue->size)) ;
        #endif
        return -1;
    }
    sem_down(&(queue->sem_sen));
    #ifdef DEBUG
        printf ("mqueue_send: mensagem adicionada a fila pela task %d \t queue->contador: %d \n", atual->tid, queue->contador) ;
    #endif
    mensagem_t men;
    men.msg = msg;
    men.prev=NULL;
    men.next=NULL;
    queue_append((queue_t**)&(queue->mensagens),(queue_t*)(&men));
    if (men.prev==NULL){
      printf("essa porra e null\n" );
    }
    printf("men.prev %d\n", men.prev);
    queue->contador+=1;
    #ifdef DEBUG
        printf ("mqueue_send: saindo com a tarefa %d e retornando com 0\n", atual->tid ) ;
    #endif
    sem_up (&queue->sem_rec);

    sem_up(&queue->sem_sen);
    return 0;
}
// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) {
    #ifdef DEBUG
        //printf ("mqueue_recv: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_recv: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    sem_down(&queue->sem_rec);
    #ifdef DEBUG
        printf ("mqueue_recv: mensagem recebida da fila de mensagens\t queue->contador: %d\n", queue->contador) ;
    #endif
    mensagem_t *a = (mensagem_t*)queue_remove((queue_t**)&(queue->mensagens),(queue_t*)(queue->mensagens));
    queue->contador-=1;
    /*
    const void *src = (queue->mensagens);

    bcopy (src, a->msg, queue->size);
    */
    msg = a->msg;
    sem_up(&queue->sem_sen);
    sem_up(&queue->sem_rec);

    return 0;
}
// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) {
    #ifdef DEBUG
        printf ("mqueue_destroy: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_destroy: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    sem_destroy(&queue->sem_rec);
    sem_destroy(&queue->sem_sen);
    queue->status=0;
    queue->mensagens=NULL;
    return 0;

}
// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) {
      #ifdef DEBUG
          printf ("mqueue_msgs: tem %d mensagens na fila de mensagens\n",queue->contador ) ;
      #endif
    return (queue->contador);
}

/*
int mqueue_send (mqueue_t *queue, void *msg) {
    #ifdef DEBUG
        printf ("mqueue_send: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_send: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    int *a =  msg; //endereco da mensagem
    //printf("a: %ld\t %ld\n", a, *a);
    //printf ("sizeof(msg) %ld\t sizeof(queue->size)%ld \n", sizeof(*a), sizeof(queue->size));
    if (sizeof(*a)!=sizeof(queue->size)){
        #ifdef DEBUG
            printf ("mqueue_send: mensagem fora do padrao\ntamanho da mensagem %ld\ttamanho aceito%ld\n", sizeof(*a), sizeof(queue->size)) ;
        #endif
        return -1;
    }
    if (queue->contador==queue->max){
        #ifdef DEBUG
            printf ("mqueue_send: task %d suspensa na fila de mensagens\n", atual->tid) ;
        #endif
        tasksMensagens+=1;
        atual->status=6;
        queue_append((queue_t**)&(queue->fila_suspensas_rec),(queue_t*)atual);
        task_switch(&dispatcher);
      	#ifdef DEBUG
            printf ("mqueue_send: task %d voltou a fila de prontas com erro -1\n", atual->tid) ;
        #endif
    }
    #ifdef DEBUG
        printf ("mqueue_send: mensagem adicionada a fila pela task %d \t queue->contador: %d \n", atual->tid, queue->contador) ;
    #endif

    mensagem_t men;
    men.msg = msg;
    men.prev=NULL;
    men.next=NULL;
    queue_append((queue_t**)&(queue->mensagens),(queue_t*)(&men));
    queue->contador+=1;
    if (queue->fila_suspensas_rec){
        printf("1\n");
        aux= (task_t*)queue_remove((queue_t**)&(queue->fila_suspensas_rec),(queue_t*)(queue->fila_suspensas_rec));
        tasksMensagens-=1;
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks+=1;
    }
    #ifdef DEBUG
        printf ("mqueue_send: saindo com a tarefa %d e retornando com 0\n", atual->tid) ;
    #endif
    return 0;
}
*/
/*int mqueue_recv (mqueue_t *queue, void *msg) {
    #ifdef DEBUG
        printf ("mqueue_recv: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_recv: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    if (queue->contador==0){
        #ifdef DEBUG
            printf ("mqueue_recv: task %d suspensa na fila de mensagens\n", atual->tid) ;
        #endif
        tasksMensagens+=1;
        atual->status=6;
        atual->cod_erro_fila_mensagem=1;
        queue_append((queue_t**)&(queue->fila_suspensas_sen),(queue_t*)atual);
        task_switch(&dispatcher);
	#ifdef DEBUG
            printf ("mqueue_recv: task %d voltou a fila de prontas com erro -1\n", atual->tid) ;
        #endif
	return -1;
    }
    #ifdef DEBUG
        printf ("mqueue_recv: mensagem recebida da fila de mensagens\t queue->contador: %d\n", queue->contador) ;
    #endif
    mensagem_t *a = (mensagem_t*)queue_remove((queue_t**)&(queue->mensagens),(queue_t*)(queue->mensagens));
    queue->contador-=1;
    /*
    const void *src = (queue->mensagens);

    bcopy (src, a->msg, queue->size);

    msg = a->msg;
    if (queue->fila_suspensas_sen){
        aux= (task_t*)queue_remove((queue_t**)&(queue->fila_suspensas_sen),(queue_t*)(queue->fila_suspensas_sen));
        tasksMensagens-=1;
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks+=1;
    }
    return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) {
    #ifdef DEBUG
        printf ("mqueue_destroy: aqui\n") ;
    #endif
    if(queue->status!=1){
        #ifdef DEBUG
            printf ("mqueue_destroy: fila de mensagens nao existe\n") ;
        #endif
        return -1;
    }
    while (queue->fila_suspensas_rec){
        aux= (task_t*)queue_remove((queue_t**)&(queue->fila_suspensas_rec),(queue_t*)(queue->fila_suspensas_rec));
        tasksMensagens-=1;
        aux->cod_erro_fila_mensagem=-1;
        #ifdef DEBUG
            printf ("mqueue_destroy: tirando a tarefa %d da fila de mensagens(rec) e passando para a fila de prontas\n", aux->tid) ;
        #endif
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks+=1;

    }
    while (queue->fila_suspensas_sen){
        aux= (task_t*)queue_remove((queue_t**)&(queue->fila_suspensas_sen),(queue_t*)(queue->fila_suspensas_sen));
        tasksMensagens-=1;
        #ifdef DEBUG
            printf ("mqueue_destroy: tirando a tarefa %d da fila de mensagens(sen) e passando para a fila de prontas\n", aux->tid) ;
        #endif
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks+=1;
    }
    queue->status=0;
    queue->mensagens=NULL;
    queue->fila_suspensas_sen=NULL;
    queue->fila_suspensas_rec=NULL;
    return 0;

}
*/
