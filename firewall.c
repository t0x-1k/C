#include <netinet/in.h>
#include <stdio.h>                 
#include <stdlib.h> 
#include <string.h>               
#include <sys/socket.h>
#include <unistd.h> 

#define PORT 8080

typedef struct {
    char ip_address[16];
    int port;
    int allow;
} FirewallRule;

FirewallRule* rules = NULL;
int ruleCount = 0;

int check_rule(const char* ip_address, int port);
void add_rule(const char* ip_address, int port, int allow);

int  main(int argc, char *argv[]) {

    int opt = 1;
    char ip[16] = {0}; 
    int port = -1;
    int allow = 1;

    if (argc == 1) {
        // If the program is run without any arguments, print usage information.
        fprintf(stderr, "Usage: %s [-a allow_ip] [-d deny_ip] [-i ip_address] [-p port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "a:d:i:p:")) != -1) {
        switch (opt) {
            case 'a':
                allow = 1;
                strncpy(ip, optarg, sizeof(ip) - 1);
                break;
            case 'd':
                allow = 0;
                strncpy(ip, optarg, sizeof(ip) - 1);
                break;
            case 'i':
                strncpy(ip, optarg, sizeof(ip) - 1);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-a allow_ip] [-d deny_ip] [-i ip_address] [-p port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if ( ip[0] != 0 && port != -1 && allow != -1) {
        add_rule(ip, port, allow);
        printf("Rule added: %s %d %s\n", ip, port, allow ? "allow" : "deny");
    } else {
        fprintf(stderr, "Invalid or incomplete rule.\n");
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;                      
    struct sockaddr_in address;   
    int addrlen = sizeof(address);
    char buffer [1024]  = {0};                         
    char *hello = "Working Server\n";                 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }    

    address.sin_family = AF_INET;                       
    address.sin_addr.s_addr = INADDR_ANY;               
    address.sin_port = htons(PORT);                      

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror( "Listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Message Sent\n");

    close(new_socket);
    close(server_fd);


    free(rules);
    rules = NULL;
    return 0;
}

int check_rule(const char* ip_address, int port) {
    for (int i = 0; i < ruleCount; i++) {
        if (strcmp(rules[i].ip_address, ip_address) == 0 && (rules[i].port == port || rules[i].port == -1)) {
            return rules[i].allow;
        }
    }
    return -1;
}

void add_rule(const char*  ip_address, int port, int allow) {
    FirewallRule* temp = realloc(rules, (ruleCount + 1)*sizeof(FirewallRule));
    if (temp == NULL) {
        perror("Failed to allocate memory for firewall rule.");
        return;
    }
    rules = temp;

    strncpy(rules[ruleCount].ip_address, ip_address, sizeof(rules[ruleCount].ip_address) - 1);
    rules[ruleCount].ip_address[sizeof(rules[ruleCount].ip_address)-1] = '\0';

    rules[ruleCount].port = port;
    rules[ruleCount].allow = allow;

    ruleCount++;
}
