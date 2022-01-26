#include "tecnicofs_client_api.h"
#define BUFFER_SIZE 41

int session_id;
int server_id;
int rx, wx;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    if (unlink(client_pipe_path) != -1){
        printf("ERROR: Couldnt delete client pipe\n");
        return -1;
    } 
    if (mkfifo(client_pipe_path, 0777) == -1){
        if (errno != EEXIST){
            printf("ERROR: Couldnt create FIFO\n");
            return -1;
        }
    }
    if(rx = open(client_pipe_path, O_RDONLY) == -1){
        printf("ERROR: Couldnt open client pipe.\n");
        return -1;
    }
    if(wx = open(server_pipe_path, O_WRONLY) == -1){
        printf("ERROR: Couldnt open server pipe.\n");
        return -1;
    }
    char buffer[BUFFER_SIZE];
    buffer[0] = TFS_OP_CODE_MOUNT;
    int size = (int)sizeof(client_pipe_path);

    for(int i = 0; i < size; i++){
        buffer[i + 1] = client_pipe_path[i];
    }

    for(int i = size; i < 40; i++){
        buffer[i + 1] = '0';
    }
    
    write(wx, buffer, sizeof(buffer));
    read(rx, session_id, sizeof(int));
    return 0;
}

int tfs_unmount() {
    /* TODO: Implement this */
    if (close(session_id) == -1){
        printf("ERROR: Couldn't close client pipe.\n");
        return -1;
    }
    if (close(server_id) == -1){
        printf("ERROR: Couldn't close client pipe.\n");
        return -1;
    }
    if (unlink(session_id) == -1){
        printf("ERROR: Couldn't unmount client session.\n");
        return -1;
    }
    return 0;
}

int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */
    int size = 1 + sizeof(session_id) + 40 + sizeof(int);
    char buffer[size];



    return -1;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
    //escrever no fifo
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

int tfs_shutdown_after_all_closed() {
    /* TODO: Implement this */
    return -1;
}
