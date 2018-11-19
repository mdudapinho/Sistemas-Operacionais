// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__
#include <ucontext.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
    struct task_t *prev;
    struct task_t *next;
    int tid ;
    ucontext_t context;
    int status ; //0= pronta; 1=fim; 2== suspensa; 3==adormecida. 4==semaforo, 5==barreira, 6==suspensa mensagens
    int prio_static;
    int aging;
    int ntick;    //numero de ticks total do processador
    int tick_total;   //numero de ticks usados pela tarefa
    int ativacoes; //numero de ativacoes da tarefa
    struct task_t *parent;
    int exit_parent;
    int adormecida; //tempo faltante para sair da fila de adormecidas
    int cod_erro_sem;
    int cod_erro_bar;
    int cod_erro_fila_mensagem;

} task_t ;
// para usar com a biblioteca de filas (cast)
// ID da tarefa
  // preencher quando necessário


// estrutura que define um semáforo
typedef struct
{
    int status; // ==1 pode usar, 0 ja foi destruida
    int valor;
    int contador;
    task_t *fila;
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
    int status;
    int valor;
    task_t *fila;
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
  int status; // ==1 pode usar, 0 ja foi destruida
  int lim;
  int contador;
  task_t *fila;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  int status;// ==1 pode usar, 0 ja foi destruida
  int contador;
  int max;
  int size;
  //task_t fila_suspensas_rec;
  //task_t fila_suspensas_sen;
  semaphore_t sem_rec;
  semaphore_t sem_sen;
  void *mensagens;

  // preencher quando necessário
} mqueue_t ;

#endif
