#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define THREADS 6
#define SIZE 2048
#define OUT "threads5.out"

int fd1;
int fd2;
int n = 0;

void* write(){
  char input[SIZE];
  //printf("%c\n\n",'A'+ args->value % 2);
  
  memset(input, 'A' + n, SIZE);
  n++;
  tfs_write(fd1, input, SIZE);
  return NULL;
}

void *read(){
    char input[SIZE];
    tfs_read(fd2, input, SIZE);
    printf("%s\n\n", input);
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
    int error;
    if (i % 2 == 0){
        error = pthread_create(&tid[i],0,&write, NULL);
    }
    else {
        error = pthread_create(&tid[i],0,&read, NULL);
    }
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
    if (i % 2 == 0)
    assert(tfs_read(fd, output, SIZE)==SIZE);
  }
  printf("Great success\n");
}

// create n threads
    // tfs_open(x, create);
    // write;
    // delete