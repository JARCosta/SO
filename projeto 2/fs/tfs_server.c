#include "operations.h"
#define MAX_SESSIONS 20
#define BUFFER_SIZE 1024
#define FREE 0
#define TAKEN 1

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
    int server_id = open(pipename, O_RDONLY);
//    printf("SERVER: server pipe opened\n");
    
    session session_list[MAX_SESSIONS];
    int free_sessions[MAX_SESSIONS] = {FREE};
    int opened_files[1024] = {-1};
    int opened_files_owners[1024];

    int a;
    while(1){
        if (a > 0) printf("***\n");
        char op_code;
        if(a = read(server_id, &op_code, sizeof(char)) > 0){
            printf("SERVER: read op_code %c\n",op_code);

            if(op_code == '1'){
                int session_id;
                char client_name[40] = {'\0'};

                if(read(server_id, &client_name, 40 * sizeof(char)) == -1){
                    printf("ERROR: reading client name\n");
                }
                printf("SERVER: client name: %s\n", client_name);
                
                session_id = -1;
                while(session_id == -1){
                    for (int i = 0; i < MAX_SESSIONS; i++){ // get first free session or wait for one, in the future, use signals
                        if (free_sessions[i] == FREE){
                            session_list[i].client_path_name = client_name;
                            session_id = i;
                            free_sessions[i] = TAKEN;
                            printf("SERVER: mounted session: %d\n", session_id);
                            break;
                        }
                    }
                }

                int return_pipe = open(client_name, O_WRONLY);
                printf("SERVER: client pipe opened\n");
                if(return_pipe  == -1){
                    printf("ERROR: Couldnt open client pipe for writing.\n");
                    return -1;
                }
                session_list[session_id].client_pipe = return_pipe;
                printf("SERVER: writing session _id...\n");
                int written = write(return_pipe, &session_id, sizeof(int));
                printf("SERVER: wrote session_id = %d\n", session_id);
                if(written <= 0){
                    printf("ERROR: writing\n");
                }
            } else if(op_code == '2'){
                int client_session, error;
                if(read(server_id, &client_session, sizeof(int)) == -1){
                    printf("ERROR: reading client name\n");
                }
                printf("SERVER: client session: %d\n", client_session);
                
                free_sessions[client_session] = FREE;
                
                if (close(session_list[client_session].client_pipe) == -1){
                    printf("ERROR: couldnt close client pipe\n");
                    free_sessions[client_session] = TAKEN;
                    error = -1;
                    write(session_list[client_session].client_pipe, &error, sizeof(int));
                    return -1;
                }
                error = 1;
                write(session_list[client_session].client_pipe, &error, sizeof(int));
                printf("SERVER: client session %d closed, state: %d\n", client_session, free_sessions[client_session]);
            } else if(op_code == '3'){
                
                open_struct input;
                read(server_id, &input, sizeof(open_struct));
                int i = 0;
                while(opened_files[i] != -1 && i < 1024) i++;
                if(opened_files[i] != -1) // wait till can open file
                opened_files[i] = tfs_open(input.name,input.flag);
                opened_files_owners[i] = input.session_id;

                session temp = session_list[input.session_id];
                int return_pipe = open(temp.client_pipe, O_WRONLY);

                //write(return_pipe, &fd, sizeof(int));

            } else if(op_code == '4'){
                
            } else if(op_code == '5'){

            } else if(op_code == '6'){

            }
        }
    }
    return 0;
}