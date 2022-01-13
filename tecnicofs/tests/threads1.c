#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define THREADS 50
#define SIZE 1024
#define OUT "threads2.out"

struct arg_struct {
    int fd;
    int value;
};

void* write(void *arguments){
  char input[SIZE];
  struct arg_struct *args = (struct arg_struct *)arguments;

  memset(input, (char)args->value + '0', SIZE);
  tfs_write(args->fd, input, SIZE);
  return NULL;
}

int main() {
  char *path = "/f1";

  char output [SIZE];

  assert(tfs_init() != -1);

  int fd = tfs_open(path, TFS_O_CREAT);
  assert(fd != -1);


  pthread_t tid[THREADS];

  for (int i = 0; i < THREADS; i++){
    struct arg_struct args;
    args.fd = fd;
    args.value = i;
    int error = pthread_create(&tid[i],0,&write, (void *)&args);
    assert(error == 0);
  }

  for (int i = 0; i < THREADS; i++){
    pthread_join (tid[i], NULL);
  }

  assert(tfs_close(fd) != -1);

  fd = tfs_open(path, 0);
  assert(fd != -1 );
  tfs_copy_to_external_fs(path, OUT);
  for(int i = 0; i < THREADS; i++){
    assert(tfs_read(fd, output, SIZE)==SIZE);
  }
  printf("Great success\n");
}

// criar threads
  // criar file x
  // escrever y
  // fechar file x



/*
thread 1:
  create(x)
  write(y)
  close(x)
thread 2:
  esperou q o file fosse criado
  verificou q o file ja foi criado
  abriu o file ou exit(ERROR)
  se abriu, escreveu y
thread 3:
  esperou q o file fosse criado
  esperou q thread 2 verificasse que file ja tinha sido criado
  verificou q file ja tinha sido criado
  abriu file ou exit(ERROR)
  se abriu, escreveu y
thread 4:
  .
  .
  .
exit(0)
wrong answer
*/