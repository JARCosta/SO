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
    
    session n_sessions[MAX_SESSIONS];
    int free_sessions[MAX_SESSIONS];

    char buffer[BUFFER_SIZE];
    char op_code;
    char client_name[40];

    while(1){
        read(server_id, buffer, BUFFER_SIZE);
        op_code = buffer[0];
        int session_id;
        switch (op_code) {
        case '1':
            session_id = -1;

            for(int i = 1; i < 41; i++){
                client_name[i-1] = buffer[i];
            }

            for (int i = 0; i < MAX_SESSIONS; i++){
                if (free_sessions[i] == FREE){
                    n_sessions[i].client_path_name = client_name;
                    
                }
                session_id = i;
                break;
            }
            int client_pipe = open(client_name, O_WRONLY);
            if(client_pipe  == -1){
                printf("ERROR: Couldnt open client pipe for writing.\n");
                return -1;
            }
            
            write(client_pipe, client_name, sizeof(client_name));
            break;
        case '2':
            for(int i = 1; i < 41; i++){
                client_name[i-1] = buffer[i];
            }

            break;
        case '3':
            break;
        case '4':
            break;
        case '5':
            break;
        case '6':
            break;
    
        default:
            break;
        }
    }
    return 0;
}