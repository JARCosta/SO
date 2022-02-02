#include "tecnicofs_client_api.h"
#define CLIENT_NAME_SIZE 40

int session_id;
int server_pipe, client_pipe;
char *client_pipe_name;
//void* message_to_server;



int send_message_to_server(int op_code, void* input_struct, size_t size_of_struct){
    size_t message_size;
    void* message_to_server;
        message_size = sizeof(char) + size_of_struct;
        message_to_server = malloc(message_size);
        char op_char = '0' + op_code;
        memcpy(message_to_server, &op_char, sizeof(char));
        memcpy(message_to_server + sizeof(char), input_struct, size_of_struct);
    size_t size = write(server_pipe, message_to_server, message_size);
//    printf("\t%d B\n",(int)size);
    return 0;
}



int mem_set_and_cpy(void* dest,char car, size_t size, char* src){
    memset(dest, car, size);
    memcpy(dest, src, strlen(src));
    return 0;
}

















int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    printf("TFS_MOUNT:\n");
    
    unlink(client_pipe_path);

// create client pipe
    if (mkfifo(client_pipe_path, 0777) == -1){
        if (errno != EEXIST){
            printf("ERROR: Couldnt create FIFO\n");
            return -1;
        }
    }

// open server pipe
    server_pipe = open(server_pipe_path, O_WRONLY);
    if(server_pipe == -1){
        printf("ERROR: Couldnt open server pipe.\n");
        return -1;
    }

    mount_struct input;
    mem_set_and_cpy(&input.client_pipe_name,'\0',sizeof(char) * CLIENT_NAME_SIZE, client_pipe_path);

    send_message_to_server(TFS_OP_CODE_MOUNT,&input,sizeof(mount_struct));

// open client pipe to read
    client_pipe = open(client_pipe_path, O_RDONLY);    
    if(client_pipe == -1){
        printf("ERROR: Couldnt open client pipe.\n");
        return -1;
    }

    size_t red = read(client_pipe, &session_id, sizeof(int));
    printf("\t pipe path: %s\n\t session_id: %d\n", input.client_pipe_name, session_id);
    return 0;
}








int tfs_unmount() {
    printf("TFS_UNMOUNT:\n");
    
    void* message_to_server = malloc(sizeof(char) + sizeof(unmount_struct));

    char op_code = '0' + TFS_OP_CODE_UNMOUNT;
    memcpy(message_to_server, &op_code, sizeof(char));

    unmount_struct input;
    input.session_id = session_id;
    memcpy(message_to_server + sizeof(char), &input, sizeof(unmount_struct));
    
    //printf("\t sending %s to server\n", (char*)message_to_server);

    int x = write(server_pipe, message_to_server, sizeof(char) + sizeof(unmount_struct));
    if(x == -1){
        printf("ERROR: writing\n");
        return -1;
    }

    int error;

    read(server_pipe, &error, sizeof(int));
    
    if (error == 1){    
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
    }

    return 0;
}









int tfs_open(char const *name, int flags) {
    printf("TFS_OPEN:\n");

    open_struct input;
    input.flag = flags;
    input.session_id = session_id;
    mem_set_and_cpy(&input.name, '\0', CLIENT_NAME_SIZE, name);

    send_message_to_server(TFS_OP_CODE_OPEN,&input,sizeof(input));
    
    int buffer;
    size_t red = read(client_pipe, &buffer, sizeof(int));
    printf("\t opened %d\n",buffer);
    return buffer;
}








int tfs_close(int fhandle) {
    printf("TFS_CLOSE:\n");

    close_struct input;
    input.session_id = session_id;
    input.fhandle = fhandle;

    send_message_to_server(TFS_OP_CODE_CLOSE,&input,sizeof(input));

    int buffer;
    size_t red = read(client_pipe, &buffer, sizeof(int));
    printf("\t closed %d\n",buffer);
    return buffer;
}








ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    printf("TFS_WRITE:\n");

    write_struct input;
    input.fhandle = fhandle;
    input.session_id = session_id;
    input.len = len;

    size_t message_size = sizeof(write_struct) + len;
    void *message = malloc(message_size);

    memcpy(message, &input, sizeof(write_struct));
    memcpy(message + sizeof(write_struct), buffer, len);
    
    send_message_to_server(TFS_OP_CODE_WRITE, message, message_size);
    
    ssize_t written;
    read(client_pipe, &written, sizeof(ssize_t));
    printf("\t wrote %d B\n", (int)written);
    return written;
}








ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    printf("TFS_READ:\n");
    
    read_struct input;
    input.fhandle = fhandle;
    input.session_id = session_id;
    input.len = len;

    send_message_to_server(TFS_OP_CODE_READ, &input, sizeof(input));

    size_t red = read(client_pipe, buffer, len);
    printf("\t read %d B\n", (int)red);

    return strlen(buffer);
}






int tfs_shutdown_after_all_closed() {
    return -1;
}
