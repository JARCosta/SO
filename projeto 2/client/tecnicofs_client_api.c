#include "tecnicofs_client_api.h"
#define CLIENT_NAME_SIZE 40

int session_id;
int server_id;
int server_pipe, client_pipe;
void* message_to_server;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */

    unlink(client_pipe_path);

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
 
    char op_code = '0' + TFS_OP_CODE_MOUNT;
    
    mount_struct input;
    printf("before message to server\n");
    memset(&input.client_pipe_name,'\0',  sizeof(char) * CLIENT_NAME_SIZE);
    memcpy(&input.client_pipe_name, client_pipe_path, sizeof(client_pipe_path));
    printf("after message to server\n");
    
    message_to_server = malloc(sizeof(char) + sizeof(mount_struct));
   
    memcpy(message_to_server, &op_code, sizeof(char));
    memcpy(message_to_server + sizeof(char), &input, sizeof(mount_struct));    
    printf("CLIENT: sending %s to server\n", (char*)message_to_server);
    
    int x = write(server_pipe, message_to_server, strlen(message_to_server));
    if(x == -1){
        printf("ERROR: writing\n");
        return -1;
    }
    printf("CLIENT: wrote %d to server\n", x);

    printf("CLIENT: opening client pipe\n");
    client_pipe = open(client_pipe_path, O_RDONLY);    
    if(client_pipe == -1){
        printf("ERROR: Couldnt open client pipe.\n");
        return -1;
    }
    printf("CLIENT: client pipe opened\n");
    int red = read(client_pipe, &session_id, sizeof(int));
    if(red == -1){
        printf("ERROR: reading\n");
        return -1;
    }
    printf("CLIENT: read %d from server\n", session_id);

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
