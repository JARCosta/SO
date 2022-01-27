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
    int free_sessions[MAX_SESSIONS] = {FREE};
    char buffer[BUFFER_SIZE];
    char op_code;
    char client_name[40] = {'\0'};
    int session_id;
    int a;

    while(1){
        if(a = read(server_id, &op_code, sizeof(char)) > 0){
            printf("SERVER: read op_code %c\n",op_code);

            if(op_code == '1'){

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
                if(write(return_pipe, &session_id, sizeof(int)) == 0){
                    printf("ERROR: writing\n");
                }
                printf("SERVER: wrote %d", session_id);
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