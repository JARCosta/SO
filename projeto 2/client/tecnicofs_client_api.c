#include "tecnicofs_client_api.h"
#define CLIENT_NAME_SIZE 40

int session_id;
int server_id;
int server_pipe, client_pipe;
void* message_to_server;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    /*
    if (unlink(client_pipe_path) == -1){
        printf("ERROR: Couldnt delete client pipe\n");
        return -1;
    } 
    */
    printf("CLIENT: creating client pipe\n");
    if (mkfifo(client_pipe_path, 0777) == -1){
        if (errno != EEXIST){
            printf("ERROR: Couldnt create FIFO\n");
            return -1;
        }
    }

    printf("CLIENT: opening server pipe\n");
    server_pipe = open(server_pipe_path, O_WRONLY);
    if(server_pipe == -1){
        printf("ERROR: Couldnt open server pipe.\n");
        return -1;
    }

    //char buffer[BUFFER_SIZE];
    //buffer[0] = '0' + (char)TFS_OP_CODE_MOUNT;
    //int size = (int)sizeof(client_pipe_path);
    
    char op_code = '0' + TFS_OP_CODE_MOUNT;
    
    mount input;
    printf("before message to server\n");
    memset(input.client_pipe_path,'\0',  sizeof(char) * CLIENT_NAME_SIZE);
    memcpy(&input.client_pipe_path, client_pipe_path, sizeof(client_pipe_path));
    printf("after message to server\n");
    message_to_server = malloc(sizeof(char) + sizeof(input));
    memcpy(message_to_server, &op_code, sizeof(char));
    memcpy(message_to_server + sizeof(char), &input, sizeof(input));

    /*
    for(int i = 0; i < size; i++){
        buffer[i + 1] = client_pipe_path[i];
    }

    for(int i = size; i < 40; i++){
        buffer[i + 1] = '0';
    }
    for(int i = 1; i < BUFFER_SIZE; i++){
        buffer[i] = 'a';
    }
    buffer[BUFFER_SIZE - 1] = '\0';
    */
    
    //printf("CLIENT: writing on server pipe: %s.\n", message_to_server);
    int x = write(server_pipe, &message_to_server, sizeof(message_to_server));
    if(x == -1){
        printf("ERROR: writing\n");
        return -1;
    }
    printf("CLIENT: wrote %d \n", x);
    printf("CLIENT: opening client pipe\n");
    client_pipe = open(client_pipe_path, O_RDONLY);
    printf("bb\n");
    
    if(client_pipe == -1){
        printf("ERROR: Couldnt open client pipe.\n");
        return -1;
    }

    if(read(client_pipe, session_id, sizeof(int)) == -1){
        printf("ERROR: reading\n");
        return -1;
    }
    
    printf("CLIENT: mounted session: %d\n", session_id);
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
