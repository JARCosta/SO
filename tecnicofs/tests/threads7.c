#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define THREADS 26
#define SIZE 1
#define OUT "threads4.out"
#define PATH "/f1"


struct arg_struct {
  char charact;
  int value;
};
char string[THREADS];
int fd[THREADS];
char buffer[SIZE];
int it;

void* threads(){
  int value = it;
  fd[it] = tfs_open(PATH,0);
  assert(tfs_read(fd,buffer,SIZE*THREADS)== SIZE*THREADS);
  printf("%s\n", buffer);
  return NULL;
}

int main() {
  for(int i = 0; i!=THREADS;i++){
    string[i] = (char)('A' + i%24);
  }
  char *PATH = "/f1";

  char output [SIZE];

  assert(tfs_init() != -1);
  for(int i = 0; i< THREADS ; i++){
    fd[i] = tfs_open(PATH, TFS_O_CREAT);
    assert(fd[i] != -1);
  }

  pthread_t tid[THREADS];

  char input[SIZE];
  for(int i = 0; i< THREADS; i++){
    for(int j = 0; j < THREADS; j++ ){
      memset(input, 'A' + j , SIZE);
      tfs_write(fd[i], input, SIZE);
    }
  }
  for(int i = 0; i< THREADS; i++) assert(tfs_close(fd[i]) != -1);

  for (int i = 0; i < THREADS; i++){
    it = i;
    int error = pthread_create(&tid[i], 0, &threads, NULL);
    assert(error == 0);
  }

  for (int i = 0; i < THREADS; i++){
    pthread_join (tid[i], NULL);
  }

  for(int i = 0; i< THREADS; i++) assert(tfs_close(fd[i]) != -1);

  tfs_copy_to_external_fs(PATH, OUT);
  
  for(int i = 0; i < THREADS; i++){
    assert(tfs_read(fd, output, SIZE)==SIZE);
  }
  printf("Great success\n");
}

/*
tfs_open(file,0)
create 4 threads
  tfs_write(var, thread_id, 3)

w/ threads: 111333222444
no threads: 222444
*/