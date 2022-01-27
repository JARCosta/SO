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
    printf("SERVER: opening server pipe\n");
    int server_id = open(pipename, O_RDONLY);
    printf("SERVER: server pipe opened\n");
    session session_list[MAX_SESSIONS];
    int free_sessions[MAX_SESSIONS];

    char buffer[BUFFER_SIZE];
    char op_code;
    char client_name[40];
    int session_id;

    while(1){
        if(read(server_id, op_code, sizeof(char)) > 0){
            printf("ERROR: reading op_code\n");
            printf("%c\n",op_code);
            if(op_code == '1'){
                printf("case 1\n");
                if(read(server_id, buffer, 40 * sizeof(char)) == -1){
                    printf("ERROR: reading client name\n");
                }
                session_id = -1;
                for(int i = 1; i < 40; i++){
                    client_name[i-1] = buffer[i];
                }
                printf("SERVER: client name: %s\n", client_name);
                while(session_id == -1){
                    for (int i = 0; i < MAX_SESSIONS; i++){ // get first free session or wait for one, in the future, use signals
                        if (free_sessions[i] == FREE){
                            session_list[i].client_path_name = client_name;
                            session_id = i;
                            printf("SERVER: mounted session: %d\n", session_id);
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
                if(write(return_pipe, (char)('0' + session_id), sizeof(session_id)) == 0){
                    printf("ERROR: writing\n");
                }
            } else if(op_code == '2'){

                /*
                char* session_id;
                int i = 0;
                for(i = 1; i < strlen(buffer); i++){
                    session_id = buffer[i];
                    session_id++;
                }
                while(i > 0)session_id--;
                */
            } else if(op_code == '3'){

            } else if(op_code == '4'){

            } else if(op_code == '5'){

            } else if(op_code == '6'){

            }
        }
    }
    return 0;
}