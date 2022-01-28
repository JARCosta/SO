#include "tecnicofs_client_api.h"
#define CLIENT_NAME_SIZE 40

int session_id;
int server_pipe, client_pipe;
char *client_pipe_name;
//void* message_to_server;

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

    client_pipe_name = client_pipe_path;

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
    
    void* message_to_server = malloc(sizeof(char) + sizeof(mount_struct));
   
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
    printf("CLIENT: unmounting\n");
    
    void* message_to_server = malloc(sizeof(char) + sizeof(unmount_struct));

    char op_code = '0' + TFS_OP_CODE_UNMOUNT;
    memcpy(message_to_server, &op_code, sizeof(char));

    unmount_struct input;
    input.session_id = session_id;
    memcpy(message_to_server + sizeof(char), &input, sizeof(unmount_struct));
    
    printf("CLIENT: sending %s to server\n", (char*)message_to_server);

    int x = write(server_pipe, message_to_server, strlen(message_to_server));
    if(x == -1){
        printf("ERROR: writing\n");
        return -1;
    }
    printf("CLIENT: wrote %d to server\n", x);

    if (close(server_pipe) == -1){
        printf("ERROR: Couldn't close client pipe.\n");
        return -1;
    }
    
    if (close(client_pipe) == -1){
        printf("ERROR: Couldn't close client pipe.\n");
        return -1;
    }
    if (unlink(client_pipe_name) == -1){
        printf("ERROR: Couldn't unmount client session.\n");
        return -1;
    }
    return 0;
}

    int send_message_to_server(int op_code, void* input_struct, ssize_t size_of_struct){
        void* message_to_server = malloc(sizeof(char) + size_of_struct);
        
        // send op_code
        char op_code = '0' + TFS_OP_CODE_UNMOUNT;
        memcpy(message_to_server, &op_code, sizeof(char));
        
        // send input_structure
        memcpy(message_to_server + sizeof(int), input_struct, sizeof(open_struct));

        return 0;
    }

int mem_set_and_cpy(void* element_in_struct,char car, ssize_t size, char* string){
    memset(element_in_struct, car, size);
    memcpy(element_in_struct, string, strlen(string));
    return 0;
}


int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */

//    void* message_to_server = malloc(sizeof(char) + (40 * sizeof(char)) + sizeof(int));
    
//    memcpy(message_to_server, &op_code, sizeof(char));

    open_struct input;
    input.flag = flags;
    input.session_id = session_id;
    mem_set_and_cpy(&input.name, '\0', 40, name);
//    memset(&input.name,'\0',  sizeof(char) * 40);
//    memcpy(&input.name, name, strlen(name) * sizeof(char));

//    memcpy(message_to_server + sizeof(int), &input, sizeof(open_struct));

    send_message_to_server(TFS_OP_CODE_OPEN,&input,sizeof(input));

    read(server_pipe, buffer, sizeof(buffer));

    receive_message_from_server();

    return 0;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
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
