#include "operations.h"
#define MAX_SESSIONS 20
#define BUFFER_SIZE 1024
#define FREE 0
#define TAKEN 1

session session_list[MAX_SESSIONS];
int free_sessions[MAX_SESSIONS] = {FREE};
int opened_files_owners[1024] = {-1};

int return_to_client(int session_id, void* buffer, size_t size){
    //session temp = session_list[session_id];
    //int return_pipe = open(temp.client_pipe, O_WRONLY);
    write(session_list[session_id].client_pipe, &buffer, size);
    return 0;
}

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
            printf("\t read op_code %c\n",op_code);

            if(op_code == '1'){
                int session_id;
                char client_name[40] = {'\0'};

                if(read(server_pipe, &client_name, 40 * sizeof(char)) == -1){
                    printf("ERROR: reading client name\n");
                }
                printf("\t client name: %s\n", client_name);
                
                session_id = -1;
                while(session_id == -1){
                    for (int i = 0; i < MAX_SESSIONS; i++){ // get first free session or wait for one, in the future, use signals
                        if (free_sessions[i] == FREE){
                            session_list[i].client_path_name = client_name;
                            session_id = i;
                            free_sessions[i] = TAKEN;
                            printf("\t mounted session: %d\n", session_id);
                            break;
                        }
                    }
                }

                int return_pipe = open(client_name, O_WRONLY);
                printf("\t client pipe opened\n");
                if(return_pipe  == -1){
                    printf("ERROR: Couldnt open client pipe for writing.\n");
                    return -1;
                }
                session_list[session_id].client_pipe = return_pipe;
                printf("\t writing session _id...\n");
                int written = write(return_pipe, &session_id, sizeof(int));
                printf("\t wrote session_id = %d\n", session_id);
                if(written <= 0){
                    printf("ERROR: writing\n");
                }
            } else if(op_code == '2'){
                unmount_struct input;
                int error;
                if(read(server_pipe, &input, sizeof(unmount_struct)) == -1){
                    printf("ERROR: reading client name\n");
                }
                printf("\t client session: %d\n", input.session_id);
                
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
                else {
                    printf("\t client session %d closed, state: %d\n", 
                    input.session_id, free_sessions[input.session_id]);
                }
            } else if(op_code == '3'){
                
                open_struct input;
                read(server_pipe, &input, sizeof(open_struct));
                printf("\t name: %s, flag: %d, session_id: %d\n", 
                input.name, input.flag, input.session_id);
                int buffer = tfs_open(input.name,input.flag);
                opened_files_owners[buffer] = input.session_id;
                printf("\t file opened: %d\n", buffer);
                /*
                int i = 0;
                while(opened_files[i] != -1 && i < 1024) i++;
                if(opened_files[i] != -1) // wait till can open file
                opened_files[i] = tfs_open(input.name,input.flag);
                opened_files_owners[i] = input.session_id;
                */
                write(session_list[input.session_id].client_pipe, &buffer, sizeof(buffer));
            } else if(op_code == '4'){

                close_struct input;
                read(server_pipe, &input, sizeof(close_struct));
                
                int owner_flag;
                if(input.session_id != opened_files_owners[input.session_id]){
                    printf("\t %d is being closed by %d, not the owner\n", input.fhandle, input.session_id);
                    owner_flag = 1;
                }
                
                int buffer = tfs_close(input.fhandle);
                write(session_list[input.session_id].client_pipe, &buffer, sizeof(buffer));
            } else if(op_code == '5'){
               // size_t input_size;
                write_struct input;
                //read(server_pipe, &input_size, sizeof(size_t));
                //printf("\t input size: %d\n", (int)input_size);
                printf("\t reading input...\n");
                read(server_pipe, &input, sizeof(write_struct));
                char buffer[input.len + 1];
                read(server_pipe, &buffer, input.len);
                printf("\t read\n");
                printf("\t input read, session_id: %d, fhandle: %d, str: %s, len: %d\n", 
                input.session_id, input.fhandle, buffer, (int)input.len);
                ssize_t written;
                buffer[input.len + 1] = '\0';
                written = tfs_write(input.fhandle, &buffer, input.len + 1);
                printf("\t written %d in %d\n", (int)written, input.fhandle);
                written -= 1;
                write(session_list[input.session_id].client_pipe, &written, sizeof(ssize_t));
            } else if(op_code == '6'){
                read_struct input;
                printf("\t reading input...\n");
                read(server_pipe, &input, sizeof(read_struct));
                printf("\t input read, session_id: %d, fhandle: %d, len: %d\n", 
                input.session_id, input.fhandle, (int)input.len);
                ssize_t red;
                char buffer[input.len];
                red = tfs_read(input.fhandle, &buffer, input.len);
                printf("\t read %s from %d\n", (char*)buffer, input.fhandle);
                write(session_list[input.session_id].client_pipe, &buffer, input.len);
            }
        }
    }
    return 0;
}