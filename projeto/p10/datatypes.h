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
    int status ; //0= pronta; 1=fim; 2== suspensa; 3==adormecida. 4==semaforo
    int prio_static;
    int aging;
    int ntick;    //numero de ticks total do processador
    int tick_total;   //numero de ticks usados pela tarefa
    int ativacoes; //numero de ativacoes da tarefa
    struct task_t *parent;
    int exit_parent;
    int adormecida; //tempo faltante para sair da fila de adormecidas
    
} task_t ;
// para usar com a biblioteca de filas (cast)
// ID da tarefa
  // preencher quando necessário


// estrutura que define um semáforo
typedef struct
{
    int contador;
    task_t *fila;
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
