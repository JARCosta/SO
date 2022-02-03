#include "operations.h"
#define MAX_SESSIONS 20
#define BUFFER_SIZE 1024
#define FREE 0
#define TAKEN 1

session session_list[MAX_SESSIONS];
int free_sessions[MAX_SESSIONS] = {FREE};
int opened_files_owners[1024] = {-1};
int server_pipe;

void* thread_func(int session_id){
    pthread_mutex_lock(&session_list[session_id].lock);
    pthread_cond_wait(&session_list[session_id].var, &session_list[session_id].lock);
    //pthread_mutex_unlock(&session_list[session_id].lock);

    char op_char = session_list[session_id].op_code;

    if (op_char == '1'){
        
    }
    else if(op_char == '2'){


    }
    else if(op_char == '3'){


    }
    else if(op_char == '4'){


    }
    else if(op_char == '5'){


    }
    else if(op_char == '6'){


    }
    else if(op_char == '7'){


    }
}

int thread_mount(){
    printf("TFS_MOUNT:\n");
    int session_id;
    char client_name[40] = {'\0'};

    if(read(server_pipe, &client_name, 40 * sizeof(char)) == -1){
        printf("ERROR: reading client name\n");
    }                
    session_id = -1;

    for (int i = 0; i < MAX_SESSIONS; i++){ // get first free session or wait for one, in the future, use signals
        if (free_sessions[i] == FREE){
            session_list[i].client_path_name = client_name;
            session_id = i;
            free_sessions[i] = TAKEN;
            printf("\t mounted session %d\n", session_id);
            break;
        }
    }

    int return_pipe = open(client_name, O_WRONLY);
    if(return_pipe  == -1){
        printf("ERROR: Couldnt open client pipe for writing.\n");
        return -1;
    }
    if (session_id != -1) session_list[session_id].client_pipe = return_pipe;
    int written = write(return_pipe, &session_id, sizeof(int));
    if(written <= 0){
        printf("ERROR: writing\n");
    }
}

int thread_unmount(int session_id){
    
    int error = 1;
    
    if (close(session_list[session_id].client_pipe) == -1){
        error = -1;
        printf("ERROR: couldnt close client pipe\n");
        write(session_list[session_id].client_pipe, &error, sizeof(int));
        return -1;
    }

    free_sessions[session_id] = FREE;
    write(session_list[session_id].client_pipe, &error, sizeof(int));
    
    printf("\t unmounted session %d\n", session_id);
}

int thread_open(){
    printf("TFS_OPEN:\n");
    open_struct input;
    read(server_pipe, &input, sizeof(open_struct));
    int buffer = tfs_open(input.name,input.flag);
    opened_files_owners[buffer] = input.session_id;
    write(session_list[input.session_id].client_pipe, &buffer, sizeof(buffer));
}

int thread_close(){
    printf("TFS_CLOSE:\n");
    close_struct input;
    read(server_pipe, &input, sizeof(close_struct));
    
    int owner_flag;
    if(input.session_id != opened_files_owners[input.session_id]){
        owner_flag = 1;
    }
    
    int buffer = tfs_close(input.fhandle);
    write(session_list[input.session_id].client_pipe, &buffer, sizeof(buffer));
    printf("\t closed %d\n",buffer);

}

int thread_write(){
    printf("TFS_WRITE:\n");

    write_struct input;
    read(server_pipe, &input, sizeof(write_struct));
    char buffer[input.len + 1];
    read(server_pipe, &buffer, input.len);
    
    ssize_t written;
    written = tfs_write(input.fhandle, &buffer, input.len);
    size_t size = write(session_list[input.session_id].client_pipe, &written, sizeof(ssize_t));
    printf("\t wrote %d B\n", written);
}

int thread_read(){
    printf("TFS_READ:\n");
    read_struct input;
    read(server_pipe, &input, sizeof(read_struct));
    ssize_t red;
    char buffer[input.len];
    red = tfs_read(input.fhandle, &buffer, input.len);
    buffer[red] = '\0';
    write(session_list[input.session_id].client_pipe, &buffer, input.len);
    printf("\t read %d B\n", red);
}

int thread_destroy_after_all_closed(){
    return -1;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);

    /* TO DO */
    if (mkfifo(pipename, 0777) == -1){
        if (errno != EEXIST){
            printf("ERROR: Couldnt create FIFO\n");
            return -1;
        }
    }
    if(tfs_init() == -1) return -1;
    for (int i; i < MAX_SESSIONS; i++){
        pthread_create(&session_list[i].thread, NULL, &write, NULL);
        pthread_cond_init(&session_list[i].var, NULL);
    }

    server_pipe = open(pipename, O_RDONLY);
//    printf("\t server pipe opened\n");

    int a;
    while(1){
        char op_code;
        a = read(server_pipe, &op_code, sizeof(char));
        if (a > 0){
            if(op_code == '1'){
                
                thread_mount();

            } else if(op_code == '2'){
                
                printf("TFS_UNMOUNT:\n");
                
                unmount_struct input;
                if(read(server_pipe, &input, sizeof(unmount_struct)) == -1){
                    printf("ERROR: reading client name\n");
                }
                                
                memcpy(session_list[input.session_id].buffer, &input, sizeof(unmount_struct));
                session_list[input.session_id].op_code = '2';

                pthread_cond_signal(&session_list[input.session_id].var);
                
                //thread_func();
                
                //thread_unmount();
            
            } else if(op_code == '3'){

                thread_open();
            
            } else if(op_code == '4'){

                thread_close();
            
            } else if(op_code == '5'){

                thread_write();
            
            } else if(op_code == '6'){

                thread_read();
            
            }else if(op_code == '7'){

                thread_destroy_after_all_closed();
            }
        }
    }
    return 0;
}