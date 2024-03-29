#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <regex.h>
#include <unistd.h>
#include <bits/getopt_core.h>

#define PORT 8080

typedef struct
{
    char ip_address[16];
    int port;
    int allow;
} FirewallRule;

FirewallRule *rules = NULL;
int ruleCount = 0;

int check_rule(const char *ip_address, int port);
void add_rule(const char *ip_address, int port, int allow);
void save_file(const char *filename);
void load_file(const char *filename);
int validate_ip(const char *ip);
int validate_port(int port);

int main(int argc, char *argv[])
{
    // try loading from this file, if it exists
    load_file("firewall_rules.txt");

    int opt = 1;
    char *outFilename = "output.txt"; // if firewall_rules.txt does not exist, create this default file
    char ip[16] = {0};
    int port = -1;
    int allow = 1;

    if (argc == 1)
    {
        // If the program is run without any arguments, print usage information.
        fprintf(stderr, "Usage: %s [-a allow_ip] [-d deny_ip] [-i ip_address] [-p port] [-o output file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //  Parse command line options using getopt
    while ((opt = getopt(argc, argv, "a:d:p:o:")) != -1)
    {
        switch (opt)
        {
        case 'a':
            allow = 1;
            strncpy(ip, optarg, sizeof(ip) - 1);
            break;
        case 'd':
            allow = 0;
            strncpy(ip, optarg, sizeof(ip) - 1);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'o':
            outFilename = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-a allow_ip] [-d deny_ip] [-p port] [-o output file]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    // function call to add rule
    add_rule(ip, port, allow);
    // function call to save file
    save_file(outFilename);

    if (ip[0] != 0 && port != -1 && allow != -1)
    {
        add_rule(ip, port, allow);
        printf("Rule added: %s %d %s\n", ip, port, allow ? "allow" : "deny");
    }
    else
    {
        fprintf(stderr, "Invalid or incomplete rule.\n");
        exit(EXIT_FAILURE);
    }
    // spinning up a server
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Working Server\n";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 2;  // Timeout in seconds
    tv.tv_usec = 0; // Timeout in microseconds
    // checking if connection is still needed, if not close connection to server
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);

    int select_result = select(server_fd + 1, &readfds, NULL, NULL, &tv);

    if (select_result > 0)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0)
        {
            perror("Read error");
            close(new_socket);
            exit(EXIT_FAILURE);
        }
        else
        {
            buffer[bytes_read] = '\0';
            printf("%s\n", buffer);
        }

        close(new_socket);
    }
    else if (select_result == 0)
    {
        printf("No incoming connections within the timeout period.\n");
    }
    else
    {
        perror("select");
        exit(EXIT_FAILURE);
    }

    close(server_fd);

    free(rules);
    rules = NULL;

    return 0;
}
// Checks if the given IP address and port match any rule defined in the `rules` array.
int check_rule(const char *ip_address, int port)
{
    for (int i = 0; i < ruleCount; i++)
    {
        if (strcmp(rules[i].ip_address, ip_address) == 0 && (rules[i].port == port || rules[i].port == -1))
        {
            return rules[i].allow;
        }
    }
    return -1;
}
// function to add rules to list
void add_rule(const char *ip_address, int port, int allow)
{
    FirewallRule *temp = realloc(rules, (ruleCount + 1) * sizeof(FirewallRule));
    if (temp == NULL)
    {
        perror("Failed to allocate memory for firewall rule.");
        return;
    }
    rules = temp;

    strncpy(rules[ruleCount].ip_address, ip_address, sizeof(rules[ruleCount].ip_address) - 1);
    rules[ruleCount].ip_address[sizeof(rules[ruleCount].ip_address) - 1] = '\0';

    rules[ruleCount].port = port;
    rules[ruleCount].allow = allow;

    ruleCount++;
}

void save_file(const char *filename)
{
    // Open the file using the provided filename for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Failed to open file for writing");
        return;
    }

    printf("Saving rules to %s\n", filename);

    // Iterate through the rules and write each to the file
    for (int i = 0; i < ruleCount; i++)
    {
        fprintf(file, "%s %d %d\n", rules[i].ip_address, rules[i].port, rules[i].allow);
    }

    if (ferror(file))
    {
        perror("Failed to write to the file");
    }

    if (fclose(file) != 0)
    {
        perror("Failed to close the file after writing");
    }
    else
    {
        // Success message for diagnostic purposes
        printf("Firewall rules successfully saved to %s\n", filename);
    }
}

void load_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Info: No existing rules to load from %s. Starting fresh.\n", filename);
        return;
    }

    char ip_address[16];
    int port, allow;

    while (fscanf(file, "%15s %d %d", ip_address, &port, &allow) == 3)
    {
        add_rule(ip_address, port, allow);
    }

    fclose(file);
    printf("Rules loaded from %s\n", filename);
}
// Using regex to ensure the ip is valid integers between 0-255
int validate_ip(const char *ip)
{
    regex_t regex;
    int ret;

    ret = regcomp(&regex, "^([0-9]{1-3}\\.){3}[0-9]{1,3}$", REG_EXTENDED);
    if (ret)
    {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    ret = regexec(&regex, ip, 0, NULL, 0);
    if (!ret)
    {
        regfree(&regex);
        return 1;
    }
    else
    {
        regfree(&regex);
        return 0;
    }
}

int validate_port(int port)
{
    return port > 0 && port <= 65535;
}