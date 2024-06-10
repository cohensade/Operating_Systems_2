#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/un.h>


#define MAX_PROG_LEN 256
int is_UDP = 0;


int setupUDS_server_dgram(const char *path) {
    int server_fd;
    struct sockaddr_un address;

    if ((server_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);

    unlink(path);
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Unix Domain Socket Datagram Server listening on %s\n", path);
    return server_fd;
}

int setupUDS_client_dgram(const char *path) {
    int client_fd;
    struct sockaddr_un server_address;

    if ((client_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, path, sizeof(server_address.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

int setupUDS_server_stream(const char *path) {
    int server_fd;
    struct sockaddr_un address;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);

    unlink(path);
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Unix Domain Socket Stream Server listening on %s\n", path);
    return server_fd;
}

int setupUDS_client_stream(const char *path) {
    int client_fd;
    struct sockaddr_un server_address;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, path, sizeof(server_address.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}


int setupUDP_client(int port, char *ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    // AF_INET: IPv4
    // SOCK_DGRAM: Datagram socket (UDP)
    // 0: Protocol, default (0)
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serv_addr, 0, sizeof(serv_addr));//set all the memory there to 0 (init memory)
    serv_addr.sin_family = AF_INET;// IPv4
    serv_addr.sin_port = htons(port);// Port number

    // Convert IPv4 address from text to binary form
    if (strcmp(ip, "localhost") == 0) {
        // If the IP address is "localhost", convert to 127.0.0.1
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    } else {
        // if not, convert the provided IP address
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }
    // Connect to the server
    // For UDP, connect() is not needed, but it allows us to use send() and recv() like in TCP
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect error");
        close(sock);
        return -1;
    }
    // Send a message
    const char *message = "hello.\n";
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(sock);
        return -1;
    }

    return sock;// return the socket

}

int setupUDP_server(int port) {
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

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    socklen_t len = sizeof(client_address);
    printf("UDP Server listening on port %d\n", port);

    int n = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_address, &len);
    if (n < 0) {
        perror("recvfrom failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("Connection established with client.\n");

    const char *message = "You are connected\n";
    sendto(server_fd, message, strlen(message), 0, (struct sockaddr *) &client_address, len);

    return server_fd;
}


int setupTCP_client(int port, char *ip) {
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
    } else {
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    const char *message = "hello.\n";
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(sock);
        return -1;
    }

    return sock;

}

int setupTCP_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

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
    if (bind(server_fd, (struct sockaddr *) &address, addrlen) < 0) {
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

int argv_tosocket(char *arg) {
    int server_fd;
    int port;

    // check if the argument starts with "TCPS" (TCP Server)
    if (strncmp(arg, "TCPS", 4) == 0) {
        port = atoi(arg + 4); // take part of the string to an integer (port number)
        return setupTCP_server(port); // create TCP server
    }
        // Check if the argument starts with "UDPS" (UDP Server)
    else if (strncmp(arg, "UDPS", 4) == 0) {
        port = atoi(arg + 4);// take part of the string to an integer (port number)
        printf("port num is: %d\n", port);
        server_fd = setupUDP_server(port);// create UDP server
        return server_fd;
    }
// Check if the argument starts with "TCPC" (TCP Client)
    else if (strncmp(arg, "TCPC", 4) == 0) {
        char *ip = strchr(arg + 4, ','); //find ','
        if (ip == NULL) { //if no ',' found
            return 1;
        }
        *ip = '\0';
        ip++;// move 1 char forward to take the port
        port = atoi(ip);//take the port
        printf("port: %d \n", port);
        int sockfd = setupTCP_client(port, "localhost");//creates TCP client
        return sockfd;
    } else if (strncmp(arg, "UDPC", 4) == 0) {
        char *ip = strchr(arg + 4, ',');//find ','
        if (ip == NULL) {//if no ',' found
            return 1;
        }

        *ip = '\0';
        ip++;// move 1 char forward to take the port
        port = atoi(ip);//take the port
        printf("portttttttttttttttttt: %d \n", port);

        int sockfd = setupUDP_client(port, "localhost");//creates UDP client
        return sockfd;
    } else if (strncmp(arg, "UDSSD", 5) == 0) {
        const char *path = arg + 5;
        return setupUDS_server_dgram(path);
    } else if (strncmp(arg, "UDSCD", 5) == 0) {
        const char *path = arg + 5;
        return setupUDS_client_dgram(path);
    } else if (strncmp(arg, "UDSSS", 5) == 0) {
        const char *path = arg + 5;
        return setupUDS_server_stream(path);
    } else if (strncmp(arg, "UDSCS", 5) == 0) {
        const char *path = arg + 5;
        return setupUDS_client_stream(path);
    }
    return 0;
}


//function to alert the user with the message "KOKO" and then exit the program.
void alert_alarm() {
    printf("KOKO\n");
    exit(0);
}

//functions to handles the flags (does dups acording to the flags)
// Function to handle -i flag
void i_flag(int fd, int is_UDP) {
    // Check if we on TCP
    if (!is_UDP) {
        // the standard input will be read from the accepted socket connection.
        int inputfd = accept(fd, NULL, NULL);
        dup2(inputfd, STDIN_FILENO);
    } else {
        // For UDP,  the standard input will be read from the UDP socket.
        dup2(fd, STDIN_FILENO);
    }

}

// Function to handle -o flag
void o_flag(int fd) {
    //output that normally go to the standard output will now go to socket referred to by fd.
    dup2(fd, STDOUT_FILENO);

}

// Function to handle -b flag
void b_flag(int fd, int is_UDP) {
    printf("lolololo\n");
    if (is_UDP) {
        printf("Cannot do -b with udp \n");
        exit(1);
    } else {

        int both_fd = accept(fd, NULL, NULL);
        dup2(both_fd, STDIN_FILENO);
        dup2(both_fd, STDOUT_FILENO);
    }
}

// Function to execute the provided program
void executeProg(char *prog) {

    char *args[MAX_PROG_LEN];
    int i = 1; // note that argv[0] is prog name

    char *token = strtok(prog, " ");

    char name[MAX_PROG_LEN];

    snprintf(name, sizeof(name), "./%s", token); // add ./
    args[0] = name;

    token = strtok(NULL, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    execvp(args[0], args);

    return; // if reached here eror occured


}


int main(int argc, char *argv[]) {

    char *to_execute; // prog to exeucte

    // to do dups on
    int input_fd = 0; //represent input
    int outpu_fd = 0; //represent output
    int both_fd = 0; // represent both

    // if a program to execute
    int e_flag = 0;

    //set time for -t arg
    int time = 0;


    char *input_arg = NULL;
    char *output_arg = NULL;
    char *both_arg = NULL;


    // parse command line arugments to sockets
    int opt;
    while ((opt = getopt(argc, argv, "e:b:i:o:t:")) != -1) {
        switch (opt) {
            case 'e': {
                e_flag = 1;
                to_execute = optarg;
                break;
            }
            case 'i': {
                input_arg = optarg;
                input_fd = argv_tosocket(input_arg);
                break;
            }
            case 'o': {
                output_arg = optarg;
                outpu_fd = argv_tosocket(output_arg);
                break;
            }
            case 'b': {
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
    printf("input set to: %s\n", input_arg);
    printf("output mode set to : %s\n", output_arg);
    printf("both mode set to : %s\n", both_arg);

    if (input_arg) {
        if ((strncmp(input_arg, "UDPS", 4) == 0) || (strncmp(input_arg, "UDSSD", 5) == 0) )   {
            is_UDP = 1;
        } else {
            is_UDP = 0;
        }
    }
    if (both_arg) {
        if ((strncmp(both_arg, "UDPS", 4) == 0) || (strncmp(both_arg, "UDSSD", 5) == 0) ) {
            is_UDP = 1;
        } else {
            is_UDP = 0;
        }
    }


    if (time > 0) {
        signal(SIGALRM, alert_alarm);
        alarm(time);
    }

    // Handle input redirection if -i flag is set
    if (input_fd) {

        i_flag(input_fd, is_UDP); // handle the i flag

    }


    // handle output redirection if -o flag is set
    if (outpu_fd) {

        o_flag(outpu_fd); // handle the o flag
    }

    // handle both input and output redirection if -b flag is set
    if (both_fd) {
        b_flag(both_fd, is_UDP); // handle the b flag
    }

    // Execute the program if -e flag is set
    if (e_flag) {
        int pid = fork();
        // child procsees execute the program
        if (pid < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            executeProg(to_execute);
            close(input_fd);
            close(outpu_fd);
            close(both_fd); //need to check if i already close those socketsss
        }

            // parent procsees wait for the child
        else {

            wait(NULL);
        }

    }
    // Execute the program if -e flag is not set
    if (!e_flag) {
        int pid = fork();
        // child procsees execute the program
        if (pid < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {

            char buff[2048];
            int read_bytes = 0;
            while (read_bytes >= 0) {
                if ((read_bytes = read(STDIN_FILENO, buff, sizeof(read_bytes))) == -1) {
                    perror("Error in read");
                    return 1;
                }

                if ((write(STDOUT_FILENO, buff, read_bytes)) == -1) {
                    perror("Error in write");
                    return 1;
                }
            }

            close(input_fd);
            close(outpu_fd);
            close(both_fd); //need to check if i already close those socketsss
        }


            // parent procsees wait for the child
        else {

            wait(NULL);
        }

    } else {


        return 1;

    }


    return 0;
}



//tcp

// -i
//first terminal: ./mync -e "ttt 123456789" -i TCPS4050
//second terminal:  nc localhost 4050

// -i -o
//first: nc -l 4455
//second: ./mync -e "./ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455
//third: nc localhost 4050

//-b
//first:   ./mync -e "./ttt 123456789" -b TCPS4050
//second:  nc localhost 4050



//udp

// -i
//first terminal: ./mync -e "ttt 123456789" -i UDPS4050
//second terminal: nc -u localhost 4050

// -i (udp) -o (tcp)
//first:  nc -l 4455     =  tcp client listen to 4455
//second: ./mync -e "ttt 123456789" -i UDPS4050 -o TCPClocalhost,4455     = input from 4050 and output will go as tcp to 4455
//third:  nc -u localhost 4050     = sends the input to the UDP server on port 4050.

// -i (udp) -o (udp)
//first:  nc -u -l 4455        =  udp client listen to 4455
//second: ./mync -e "ttt 123456789" -i UDPS4050 -o UDPClocalhost,4455          = input from 4050 and output will go as udp to 4455
//third: nc -u localhost 4050        = sends the input to the UDP server on port 4050.



/* unix domain
 *
 * path = /home/daniel/Desktop/Operating Systems/work2/exe6
 *
 * -i case
 *
 *
 *
 *
 *
 * -o case
 *
 *
 *
 *
 *
 *
 * -b case
 *
 *           first terminal:  ./mync -b UDSSS/home/daniel/Desktop/Operating Systems/work2/exe6
 *           second terminal: ./mync -b UDSCS/home/daniel/Desktop/Operating Systems/work2/exe6
 *
 *
 */




// sudo kill $(lsof -t -i:8080)