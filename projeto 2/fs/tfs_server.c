#include "operations.h"
#define MAX_SESSIONS 20
#define BUFFER_SIZE 1024
#define FREE 0
#define TAKEN 1

session session_list[MAX_SESSIONS];
int free_sessions[MAX_SESSIONS] = {FREE};
int opened_files_owners[1024] = {-1};

int thread_func(){
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

    int server_pipe = open(pipename, O_RDONLY);
//    printf("\t server pipe opened\n");

    int a;
    while(1){
        if (a > 0) printf("");
        char op_code;
        a = read(server_pipe, &op_code, sizeof(char));
        if (a > 0){
            if(op_code == '1'){
                printf("TFS_MOUNT:\n");
                int session_id;
                char client_name[40] = {'\0'};

                if(read(server_pipe, &client_name, 40 * sizeof(char)) == -1){
                    printf("ERROR: reading client name\n");
                }                
                session_id = -1;
                while(session_id == -1){
                    for (int i = 0; i < MAX_SESSIONS; i++){ // get first free session or wait for one, in the future, use signals
                        if (free_sessions[i] == FREE){
                            session_list[i].client_path_name = client_name;
                            session_id = i;
                            free_sessions[i] = TAKEN;
                            printf("\t mounted session %d\n", session_id);
                            break;
                        }
                    }
                }

                int return_pipe = open(client_name, O_WRONLY);
                if(return_pipe  == -1){
                    printf("ERROR: Couldnt open client pipe for writing.\n");
                    return -1;
                }
                session_list[session_id].client_pipe = return_pipe;
                int written = write(return_pipe, &session_id, sizeof(int));
                if(written <= 0){
                    printf("ERROR: writing\n");
                }
            } else if(op_code == '2'){
                printf("TFS_UNMOUNT:\n");
                unmount_struct input;
                int error;
                if(read(server_pipe, &input, sizeof(unmount_struct)) == -1){
                    printf("ERROR: reading client name\n");
                }
                
                error = 1;
                free_sessions[input.session_id] = FREE;
                write(session_list[input.session_id].client_pipe, &error, sizeof(int));
                
                if (close(session_list[input.session_id].client_pipe) == -1){
                    printf("ERROR: couldnt close client pipe\n");
                    free_sessions[input.session_id] = TAKEN;
                    error = -1;
                    write(session_list[input.session_id].client_pipe, &error, sizeof(int));
                    return -1;
                }
                printf("\t unmounted session %d\n", input.session_id);



            } else if(op_code == '3'){
                printf("TFS_OPEN:\n");
                open_struct input;
                read(server_pipe, &input, sizeof(open_struct));
                int buffer = tfs_open(input.name,input.flag);
                opened_files_owners[buffer] = input.session_id;
                write(session_list[input.session_id].client_pipe, &buffer, sizeof(buffer));
            
            
            
            } else if(op_code == '4'){
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
            
            
            } else if(op_code == '5'){
                printf("TFS_WRITE:\n");

                write_struct input;
                read(server_pipe, &input, sizeof(write_struct));
                char buffer[input.len + 1];
                read(server_pipe, &buffer, input.len);
                
                ssize_t written;
                buffer[input.len + 1] = '\0';
                written = tfs_write(input.fhandle, &buffer, input.len + 1);
                written -= 1;
                size_t size = write(session_list[input.session_id].client_pipe, &written, sizeof(ssize_t));
                printf("\t wrote %d B\n", written);
            
            } else if(op_code == '6'){
                printf("TFS_READ:\n");
                read_struct input;
                read(server_pipe, &input, sizeof(read_struct));
                ssize_t red;
                char buffer[input.len];
                red = tfs_read(input.fhandle, &buffer, input.len);
                write(session_list[input.session_id].client_pipe, &buffer, input.len);
                printf("\t read %d B\n", red);
            }
        }
    }
    return 0;
}