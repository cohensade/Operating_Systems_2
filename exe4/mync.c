#include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <getopt.h>
    #include <signal.h>

    int is_UDP = 0;
    #define MAX_PROG_LEN 256


    int setupUDP_client(int port , char* ip){
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 address from text to binary form
    if (strcmp(ip, "localhost") == 0) {
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    } else {
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect error");
        close(sock);
        return -1;
    }
    const char *message = "hello UDP server.\n";
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(sock);
        return -1;
    }

    return sock;

    }

    int setupUDP_server(int port){
    int server_fd;
    struct sockaddr_in address, client_address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow address and port reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    socklen_t len = sizeof(client_address);
    printf("UDP Server listening on port %d\n", port);

    int n = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &len);
    if (n < 0) {
        perror("recvfrom failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("Connection established with client.\n");

    const char *message = "You are connected\n";
    sendto(server_fd, message, strlen(message), 0, (struct sockaddr *)&client_address, len);
     
    return server_fd;
}

    

    int setupTCP_client(int port , char* ip){
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
        //printf("the port is: %d\n" , port);
        
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

        const char *message = "hello UDP server.\n";
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(sock);
        return -1;
    }

        return sock;

    }

    int setupTCP_server(int port){
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
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt))) {
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
        int server_fd;
        int port;

        // arg is a server
        if( strncmp( arg , "TCPS", 4) == 0){    
             port = atoi(arg+4);
             is_UDP =0;
            return setupTCP_server(port);
        }

        else if( strncmp( arg , "UDPS", 4) == 0){    
             port = atoi(arg+4);
            printf("port num is: %d\n", port);
            server_fd = setupUDP_server(port);
           is_UDP =1;
        
            return server_fd;
        }

        else if ( strncmp ( arg , "TCPC" , 4) == 0 ) { 
            char* ip = strchr(arg+4 , ',');
            if ( ip == NULL ){
                return 1; 
            }
            *ip = '\0';
            ip++;
             port = atoi(ip);
                printf("port: %d \n", port);
            is_UDP =0;
            int sockfd  = setupTCP_client(port ,"localhost");
            return sockfd;
        }

        else if ( strncmp ( arg , "UDPC" , 4) == 0 ) { 
            char* ip = strchr(arg+4 , ',');
            if ( ip == NULL ){
                return 1; 
            }
            is_UDP = 1;
            *ip = '\0';
            ip++;
            port = atoi(ip);
                printf("portttttttttttttttttt: %d \n", port);
                printf("ippppppppppppppppppp: %s \n", ip);
        
            int sockfd  = setupUDP_client(port ,"localhost");
            return sockfd;
        }


        return 0;
    }


    void alert_alarm(int sig) {
    printf("KOKO\n");
    exit(0);
}


    // Function to handle -i flag
    void i_flag(int fd, int is_UDP){
    printf("ata po?\n");
    if(!is_UDP){
    int output_fd = accept(fd , NULL ,NULL );
    dup2(output_fd , STDIN_FILENO);
    }else {
    dup2(fd , STDIN_FILENO);
    }
   
    }

    // Function to handle -o flag
    void o_flag(int fd, int is_UDP){
    printf("polopolopopo\n");
    if (is_UDP)
    {
        int fdddd =setupUDP_client( 4050 , "localhost");
        dup2(STDERR_FILENO , fdddd);
        //dup2(fdddd , STDOUT_FILENO);
        //dup2(STDERR_FILENO , fdddd);
      //  dup2(fdddd , STDOUT_FILENO);
       // dup2(fdddd , STDERR_FILENO);
  //  int output_fd = connect(fd,NULL,NULL);
  // dup2(output_fd ,fd);
//dup2(output_fd , STDOUT_FILENO);
   //dup2(fd, STDOUT_FILENO);
    printf("zevel \n");
    }else{
    
    int input_fd = accept(fd , NULL ,NULL );
    dup2(input_fd ,STDERR_FILENO);
    dup2(input_fd , STDOUT_FILENO);
    }
    }
    // Function to handle -b flag
    void b_flag(int fd, int is_UDP){
    printf("lolololo\n");
    if (is_UDP)
    {
        printf("Cannot do -b with udp \n");
        exit(1);
    }else{
    
    int both_fd = accept(fd , NULL ,NULL );
    dup2(both_fd , STDIN_FILENO);
    dup2(both_fd , STDERR_FILENO);
    dup2(both_fd , STDOUT_FILENO); 
    }
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
        int input_fd=0; //represent input 
        int outpu_fd=0; //represent output
        int both_fd=0; // represent both

        // if a program to execute
        int e_flag = 0;
        
        //set time for -t arg
        int time =0;


        char* input_arg = NULL;
        char* output_arg = NULL ;
        char* both_arg = NULL;


        // parse command line arugments to sockets
        int opt;
        while( (opt = getopt(argc,argv ,"e:b:i:o:t:") ) != -1 ){
            switch (opt){
            case 'e':{
                e_flag =1;
                to_execute = optarg;
                break;
            }
            case 'i':{
                input_arg = optarg;
                input_fd = argv_tosocket(input_arg);
                break;
            }
            case 'o':{
                output_arg = optarg;
                outpu_fd = argv_tosocket(output_arg);
                break;
            }
            case 'b':{
                both_arg = optarg;
                both_fd = argv_tosocket(both_arg);
                break;
            }
            case 't':
                time = atoi(optarg);
                break;

            default:
                break;

            }

        }
    //prints the redirections (testing)
        printf("input set to: %s\n" , input_arg ) ;
        printf("output mode set to : %s\n" , output_arg );
        printf("both mode set to : %s\n" , both_arg );
        
          if (time > 0)
          {
            signal(SIGALRM, alert_alarm);
            alarm(time);
          }

        
        // does dups acording to the flags


        // Handle input redirection if -i flag is set
        if ( input_fd ){   
            
            i_flag(input_fd, is_UDP) ; // handle the i flag 
        
        }

        
    // handle output redirection if -o flag is set
        if ( outpu_fd){
            
            o_flag(outpu_fd, is_UDP) ; // handle the o flag 
        }

    // handle both input and output redirection if -b flag is set
        if( both_fd){
            b_flag(both_fd, is_UDP); // handle the b flag 
        }
        
        // Execute the program if -e flag is set
        if(e_flag) {
        int pid = fork();
        // child procsees execute the program   
        if( pid < 0 ) {
            perror("fork");
            return 1;
        } 
        else if (pid == 0) { 
            executeProg(to_execute);
            close(input_fd); close(outpu_fd); close(both_fd); //need to check if i already close those socketsss
        }
        
        // parent procsees wait for the child 
        else{

            wait(NULL);
        }

        }
    // Execute the program if -e flag is not set
        if(!e_flag) {
        int pid = fork();
        // child procsees execute the program   
        if( pid < 0 ) {
            perror("fork");
            return 1;
        }
        else if(pid == 0){
        
            char buff[2048];
            int read_bytes = 0;
            while (read_bytes >= 0)
            {
            if ((read_bytes = read(STDIN_FILENO, buff, sizeof(read_bytes))) == -1) 
            {
                perror("Error in read");
                return 1;
            }
            
                if((write(STDOUT_FILENO, buff, read_bytes)) == -1){
                    perror("Error in write");
                    return 1;
                }
            }

            close(input_fd); close(outpu_fd); close(both_fd); //need to check if i already close those socketsss
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
    //           nc localhost 4455


    //-b
    //./mync -e "./ttt 123456789" -b TCPS4050 



    // sudo kill $(lsof -t -i:8080)