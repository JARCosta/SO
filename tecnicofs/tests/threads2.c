#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define THREADS 50
#define SIZE 1024
#define PATH "/f1"

struct arg_struct {
  int value;
};

void* func(void *arguments){
  char input[SIZE];
  struct arg_struct *args = (struct arg_struct *)arguments;
  
  int fd = tfs_open(PATH, TFS_O_CREAT);
  assert(fd != -1);

  memset(input, (char)args->value + '0', SIZE);
  tfs_write(fd, input, SIZE);
  tfs_close(fd);
  return 0;
}

int main() {
  char output [SIZE];
  pthread_t tid[THREADS];

  assert(tfs_init() != -1);

  for (int i = 0; i < THREADS; i++){
    struct arg_struct args;
    args.value = i;
    pthread_create(&tid[i],0,&func, (void *)&args);
  }

  for (int i = 0; i < THREADS; i++){
  pthread_join (tid[i], NULL);
  }


  int fd = tfs_open(PATH, 0);
  assert(fd != -1 );
  
  tfs_copy_to_external_fs(PATH, "out");

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




// criar file x
// criar threads
  // escrever y
  // fechar x







/*
open(x)
thread 1:
  escreveu y
  close(x)
thread 2:
  exit(ERROR), file nao esta aberto
thread 3:
  .
  .
  .
exit(0)
wrong answer
*/