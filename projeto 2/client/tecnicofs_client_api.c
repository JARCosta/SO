#include "tecnicofs_client_api.h"
#define CLIENT_NAME_SIZE 40

int session_id;
int server_pipe, client_pipe;
char *client_pipe_name;
//void* message_to_server;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {

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
    printf("CLIENT: unmounting\n");
    
    void* message_to_server = malloc(sizeof(char) + sizeof(unmount_struct));

    char op_code = '0' + TFS_OP_CODE_UNMOUNT;
    memcpy(message_to_server, &op_code, sizeof(char));

    unmount_struct input;
    input.session_id = session_id;
    memcpy(message_to_server + sizeof(char), &input, sizeof(unmount_struct));
    
    //printf("CLIENT: sending %s to server\n", (char*)message_to_server);

    int x = write(server_pipe, message_to_server, sizeof(char) + sizeof(unmount_struct));
    if(x == -1){
        printf("ERROR: writing\n");
        return -1;
    }
    printf("CLIENT: wrote %d to server\n", x);

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

int send_message_to_server(int op_code, void* input_struct, size_t size_of_struct){
    size_t message_size;
    void* message_to_server;
    /*if (op_code == TFS_OP_CODE_WRITE){
        message_size = sizeof(char) + sizeof(size_t) + size_of_struct;
        message_to_server = malloc(message_size);
        printf("CLIENT: sizeof struct: %d\n", (int)size_of_struct);
        char op_char = '0' + op_code;
        memcpy(message_to_server, &op_char, sizeof(char));
        memcpy(message_to_server + sizeof(char), &size_of_struct, sizeof(size_t));
        memcpy(message_to_server + sizeof(size_t) + sizeof(char), input_struct, size_of_struct);
        printf("CLIENT: message: %s\n", (char*)message_to_server);
    }*/
    //else{
        message_size = sizeof(char) + size_of_struct;
        message_to_server = malloc(message_size);
        printf("CLIENT: sizeof struct: %d\n", (int)message_size);
        char op_char = '0' + op_code;
        memcpy(message_to_server, &op_char, sizeof(char));
        memcpy(message_to_server + sizeof(char), input_struct, size_of_struct);

    //}
    write(server_pipe, message_to_server, message_size);
//    write(server_pipe, message_to_server, strlen(message_to_server));
    return 0;
}

int mem_set_and_cpy(void* element_in_struct,char car, size_t size, char* string){
    memset(element_in_struct, car, size);
    memcpy(element_in_struct, string, strlen(string));
    return 0;
}

int tfs_open(char const *name, int flags) {
    printf("CLIENT: tfs_open\n");

    open_struct input;
    input.flag = flags;
    input.session_id = session_id;
    mem_set_and_cpy(&input.name, '\0', 40, name);
    printf("CLIENT: file name: %s, flag: %d\n", name, input.flag);

    printf("CLIENT: sending input...\n");
    send_message_to_server(TFS_OP_CODE_OPEN,&input,sizeof(input));
    printf("CLIENT: input sent\n");
    int buffer;
    printf("CLIENT: reading from client pipe...\n");
    read(client_pipe, &buffer, sizeof(int));
    printf("CLIENT: fhandle received: %d\n", buffer);
    return buffer;
}

int tfs_close(int fhandle) {
    printf("CLIENT: tfs_close\n");

    close_struct input;
    input.session_id = session_id;
    input.fhandle = fhandle;
    printf("CLIENT: fhandle: %d\n", fhandle);

    printf("CLIENT: sending input...\n");
    send_message_to_server(TFS_OP_CODE_CLOSE,&input,sizeof(input));
    printf("CLIENT: input sent\n");

    int buffer;
    printf("CLIENT: reading from client pipe...\n");
    read(client_pipe, &buffer, sizeof(int));
    printf("CLIENT: read: %d\n", buffer);
    return buffer;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    printf("CLIENT: tfs_write\n");

    write_struct input;
    input.fhandle = fhandle;
    input.session_id = session_id;
    input.len = len;

    size_t message_size = sizeof(char) + sizeof(write_struct) + len;
    void *message = malloc(message_size);

    char op_char = '0' + TFS_OP_CODE_WRITE;
    memcpy(message, &op_char, sizeof(char));
    memcpy(message + sizeof(char), &input, sizeof(write_struct));
    memcpy(message + sizeof(char) + sizeof(write_struct), buffer, len);
    printf("CLIENT: sending input...\n");
    write(server_pipe, message, message_size);
    //mudar a o buffer para buffer[len];
    //send_message_to_server(TFS_OP_CODE_WRITE, &input, message_size);

    printf("CLIENT: input sent: fhandle: %d, session_id: %d, str: %s, length: %d\n", 
    input.fhandle, input.session_id, (char*)buffer, (int)input.len);
    

    ssize_t written;
    printf("CLIENT: reading from client pipe...\n");
    read(client_pipe, &written, sizeof(ssize_t));
    printf("CLIENT: written %d in file\n", (int)written);
    return written;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    printf("CLIENT: tfs_read\n");
    
    read_struct input;
    input.fhandle = fhandle;
    input.session_id = session_id;
    input.len = len;
    printf("CLIENT: len = %d\n", (int)len);

    printf("CLIENT: sending input...\n");
    send_message_to_server(TFS_OP_CODE_READ, &input, sizeof(input));

    printf("CLIENT: reading from client pipe...\n");
    size_t red = read(client_pipe, buffer, len);
    printf("CLIENT: read %d from file\n", (int)red);

    return strlen(buffer);
}

int tfs_shutdown_after_all_closed() {
    return -1;
}
