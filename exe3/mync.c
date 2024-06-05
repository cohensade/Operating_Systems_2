#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#define MAX_PROG_LEN 256

int setUp_client(int port , char* ip){
     int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(serv_addr)); // clean the struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // Convert IPv4 address from text to binary form
    if (strcmp(ip, "localhost") == 0) {
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            
            perror("Invalid address/ Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    } 
    else {
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;

}

int setUp_server(int port){
  int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address); // Fix type mismatch

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options on the socket to allow address and port reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Define the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    return server_fd;
}

int argv_tosocket(char* arg){

    // arg is a server
    if( strncmp( arg , "TCPS", 4) == 0){    
        int port = atoi(arg+4);
        fprintf(stderr,"test_o");
        return setUp_server(port);
    }

    else if ( strncmp ( arg , "TCPC" , 4) == 0 ) { 
       
        char* ip = strchr(arg+4 , ',');
        if ( ip == NULL ){
            return 1; 
        }

        *ip = '\0';
        ip++;
         int port = atoi(ip);
          
        
       
        int sockfd  = setUp_client(4050 ,"localhost");
        return sockfd;

    }


    return 0;
}


// Function to handle -i flag
void i_flag(int fd){
    fprintf(stderr,"ata po?");
   int output_fd = accept(fd , NULL ,NULL );
   dup2(output_fd , STDIN_FILENO);

}

// Function to handle -o flag
void o_flag(int fd){
    fprintf(stderr,"polopolopopo");
  int input_fd = accept(fd , NULL ,NULL );
  dup2(input_fd ,STDERR_FILENO);
  dup2(input_fd , STDOUT_FILENO);
}

// Function to handle -b flag
void b_flag(int fd){
int both_fd = accept(fd , NULL ,NULL );
dup2(both_fd , STDIN_FILENO);
dup2(both_fd , STDERR_FILENO);
dup2(both_fd , STDOUT_FILENO); 
}


// Function to execute the provided program
void executeProg(char* prog){

    char *args[MAX_PROG_LEN];
    int i = 1; // note that argv[0] is prog name 

    char* token = strtok(prog, " ");

    char  name[MAX_PROG_LEN];

    snprintf(name , sizeof(name) ,"./%s", token); // add ./ 
    args[0] = name;

    token = strtok(NULL ," ");
    while(token != NULL){
        args[i++] = token;
        token = strtok(NULL," ");
    }
    args[i] = NULL;

    execvp(args[0] , args);
    
    return; // if reached here eror occured 


}

int main(int argc , char* argv[]){

    char* to_execute; // prog to exeucte 

    // to do dups on 
    int input_fd; //represent input 
    int outpu_fd; //represent output
    int both_fd; // represent both

    // if a program to execute
    int e_flag = 0;


    char* input_arg = NULL;
    char* output_arg = NULL ;
    char* both_arg = NULL;


    // parse command line arugments to sockets
    int opt;
    while( (opt = getopt(argc,argv ,"e:b:i:o:") ) != -1 ){
        switch (opt){
        case 'e':
            e_flag =1;
            to_execute = optarg;
            break;
        
        case 'i':
            input_arg = optarg;
            input_fd = argv_tosocket(input_arg);
            break;

        case 'o':
            output_arg = optarg;
            outpu_fd = argv_tosocket(output_arg);
            break;

        case 'b':
            both_arg = optarg;
            both_fd = argv_tosocket(both_arg);
            break;

        default:
            break;,

        }

    }

    printf("input set to: %s\n" , input_arg ) ;
    printf("output mode set to : %s\n" , output_arg );
    printf("both mode set to test: %s\n" , both_arg );
      
    // does dups acording to the flags
    // Handle input redirection if -i flag is set
    if ( input_fd){   
        
        i_flag(input_fd) ; // handle the i flag 
      
    }

    
// Handle output redirection if -o flag is set
    if ( outpu_fd){
        
        o_flag(outpu_fd) ; // handle the o flag 
    }

 // Handle both input and output redirection if -b flag is set
    if( both_fd){
        b_flag(both_fd); // handle the b flag 
    }
    
    // Execute the program if -e flag is set
    if(e_flag) {
      int pd = fork();
      // child procsees execute the program   
      if( pd == 0 ) {
        
        executeProg(to_execute);
     }
      // parent procsees wait for the child 
      else{

        wait(NULL);
      }

    }

    else{


        return 1;

    }
  




    return 0;
}

// -i first terminal
    //        ./mync -e "./ttt 123456789" -i TCPS4050
    // -i second terminal
    //         telnet localhost 4050


// -i -o first terminal
    //       nc -l 4050
    // -i -o second terminal
    //         ./mync -e "./ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455
    // -i -o third terminal
    //           nc localhost 4050


    //-b
    //./mync -e "./ttt 123456789" -b TCPS4050 



    //kill $(lsof -t -i:8080)

