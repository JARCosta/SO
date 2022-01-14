#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define THREADS 12
#define SIZE 2
#define OUT "threads4.out"
#define PATH "/f1"


struct arg_struct {
  char charact;
  int value;
};
char string[THREADS*SIZE];
int fd[THREADS];
int FD;
char buffer[SIZE*THREADS];
int it;

void* threads(){
  int value = it;
  //printf("%i\n",value);
  int thread_file_handle = tfs_open(PATH,0);
  tfs_read(thread_file_handle,buffer, (size_t)(SIZE*value));
  tfs_close(thread_file_handle);
  printf("%s\n", buffer);
  return NULL;
}

int main() {

  assert(tfs_init() != -1);

  FD = tfs_open(PATH, TFS_O_CREAT);
  assert(FD != -1);

  pthread_t tid[THREADS];

  for(int i = 0; i!=THREADS;i++){
    for(int j = 0; j<SIZE;j++){
      string[i*SIZE+j] = (char)('A' + i%24);
    }
  }
  printf("input: %s\n",string);

  tfs_write(fd[1], string, (size_t)(SIZE * THREADS));
  printf("%s\n",string);
  assert(tfs_close(FD) != -1);

  for (int i = 0; i < THREADS; i++){
    it = i;
    int error = pthread_create(&tid[i], 0, &threads, NULL);
    assert(error == 0);
  }

  for (int i = 0; i < THREADS; i++){
    pthread_join (tid[i], NULL);
  }

  //for(int i = 0; i< THREADS; i++) assert(tfs_close(fd[i]) != -1);

  tfs_copy_to_external_fs(PATH, OUT);
  /*
  char output [SIZE];
  for(int i = 0; i < THREADS; i++){
    assert(tfs_read(fd[i], output, SIZE)==SIZE);
  }
  */
  printf("Great success\n");
}

/*
tfs_open(file,0)
create 4 threads
  tfs_write(var, thread_id, 3)

w/ threads: 111333222444
no threads: 222444
*/