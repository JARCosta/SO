#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

#define N 4


typedef struct {
  int saldo;
  int numMovimentos;
  pthread_mutex_t lock;
  /* outras variáveis,ex. nome do titular, etc. */
} conta_t;

//Variaveis globais
conta_t c;


int depositar_dinheiro(conta_t* conta, int valor) {
  if (valor < 0)
    return -1;
  conta->saldo += valor;
  conta->numMovimentos ++;
  return valor;
}

int levantar_dinheiro(conta_t* conta, int valor) {
  if (valor < 0)
    return -1;
  
  if (conta->saldo >= valor) {
    conta->saldo -= valor;
    conta->numMovimentos ++;
  } else{
    valor = -1;
  }
  return valor;
}

void consultar_conta(conta_t* conta) {
  int s, n;

  s = conta->saldo;
  n = conta->numMovimentos;
  
  printf("Consulta: saldo=%d, #movimentos=%d\n", s, n);
}


void *fnAlice(void *arg) {
  int m = *((int*)arg);
  int total = 0;
  int r;
  pthread_mutex_lock(&c.lock);
  for (int i = 0; i<m; i++) {
    r = depositar_dinheiro(&c, 1);
    if (r != -1)
      total += r;
  }
  pthread_mutex_unlock(&c.lock);
  printf("Alice depositou no total: %d\n", total);
  return NULL;
}


void *fnBob(void *arg) {
  int m = *((int*)arg);
  int total = 0;
  int r;

  pthread_mutex_lock(&c.lock);
  for (int i = 0; i<m; i++) {
    r = levantar_dinheiro(&c, 1);
    if (r != -1)
      total += r;
  }
  pthread_mutex_unlock(&c.lock);

  printf("Bob gastou no total: %d\n", total);
  return NULL;
}


int main(int argc, char** argv) {

  pthread_t tid[2];
  
  int m;

  pthread_mutex_init(&c.lock,NULL);
  if (argc > 1)
    m = atoi(argv[1]);
  else
    m = 0;
  
  c.saldo = 0;
  c.numMovimentos = 0;
  
  if (pthread_create (&tid[0], NULL, fnAlice, (void*)&m) != 0)
    exit(EXIT_FAILURE);
  if (pthread_create (&tid[1], NULL, fnBob, (void*)&m) != 0)
    exit(EXIT_FAILURE);
  
  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);

  pthread_mutex_destroy(&c.lock);
  printf("História chegou ao fim\n");
  consultar_conta(&c);

  exit(EXIT_SUCCESS);
}

