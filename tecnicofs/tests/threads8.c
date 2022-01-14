#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define SIZE 1
#define NUMBER_OF_THREADS 5
#define OUTPUT_SIZE 300
#define SIZE_OUTPUT 2

struct thread_info{
    char *folder_path;
    char *input;
    char *output;
};

void* thread_func(void *arg){
    struct thread_info *info = arg;
    char* path = info->folder_path;
    for (int i=0; i!=100; i++){
        int fd = tfs_open(path, TFS_O_CREAT);
        assert(fd!=-1);
        char* input_to_write = info->input;
        char* output = info->output;
        assert(tfs_write(fd, input_to_write, SIZE)!=-1);
        assert(tfs_close(fd)!=-1);
        fd = tfs_open(path, 0);
        assert(fd!=-1);
        assert(tfs_read(fd, output, sizeof(output)-1)!=-1);
        output[1]='\0';

        assert(strlen(output)==1);

        assert(strcmp(input_to_write, output));
        assert(tfs_close(fd)!=-1);
    }
    return 0;
}

int main() {
    char *path1 = "/f1";
    char *path2 = "/f2";
    char *path3 = "/f3";
    char *path4 = "/f4";
    char *path5 = "/f5";

    struct thread_info *arg[5];

    pthread_t thread[5];

    char input1[SIZE];
    char input2[SIZE];
    char input3[SIZE];
    char input4[SIZE];
    char input5[SIZE];
    char output1[SIZE_OUTPUT];
    char output2[SIZE_OUTPUT];
    char output3[SIZE_OUTPUT];
    char output4[SIZE_OUTPUT];
    char output5[SIZE_OUTPUT];
    memset(input1, 'A', 1);
    memset(input2, 'B', 1);
    memset(input3, 'C', 1);
    memset(input4, 'D', 1);
    memset(input5, 'E', 1);

    for (int i=0; i<5; i++){
        arg[i] = calloc(1, sizeof(struct thread_info));
    }

    arg[0]->input=input1;
    arg[1]->input=input2;
    arg[2]->input=input3;
    arg[3]->input=input4;
    arg[4]->input=input5;

    arg[0]->folder_path=path1;
    arg[1]->folder_path=path2;
    arg[2]->folder_path=path3;
    arg[3]->folder_path=path4;
    arg[4]->folder_path=path5;

    arg[0]->output=output1;
    arg[1]->output=output2;
    arg[2]->output=output3;
    arg[3]->output=output4;
    arg[4]->output=output5;

    assert(tfs_init() != -1);

    for (int tnumber=0; tnumber<NUMBER_OF_THREADS; tnumber++){
        assert(pthread_create(&thread[tnumber], NULL, thread_func, arg[tnumber])!=-1);
    }

    for (int tnumber=0; tnumber<NUMBER_OF_THREADS; tnumber++){
        assert(pthread_join(thread[tnumber], NULL)!=-1);
    }

    assert(tfs_destroy()!=-1);
    printf("Successful test.\n");
    return 0;
}