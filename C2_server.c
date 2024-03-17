#define _GNU_SOURCE
#include <arpa/inet.h>              // address manipulation
#include <fcntl.h>                  // file distriptor manipulation
#include <stdio.h>                  // file operations
#include <stdlib.h>                 // memory allocation / deallocation, exit()
#include <netinet/ip_icmp.h>        // icmp header
#include <netinet/in.h>             // internet protocol definitions
#include <netinet/ip.h>             // ip header               
#include <sys/socket.h>             // socket creation and manipulation
#include <sys/types.h>
#include <unistd.h>                 // for getpid()

void display_help() {
    printf("Usage: [options]\n");
    printf("\t-i <IP Address> Specify IP Address\n");
    printf("\t-m <MAC Address> Specify MAC Address\n");
    printf("\t-l <Connected Hosts> List of all connected hosts\n");
    printf("\t-s <Sending Data> Specify Data to be sent\n");
    printf("\t-r <Receiving Data> Data being received from hosts\n");

    }

void mcAddress(unsigned long macAddress) {
    printf("MAC Address: ");
    for (int i = 5; i >= 0; i--) {
        printf("%02X", (unsigned int)(macAddress >> (i * 8) & 0xFF ));
        if (i > 0) {
            printf(":");
        }
    }
    printf("\n");
}

unsigned long parse_mcAddress(const char* macString) {
    unsigned long macAddr = 0;
    int bytes_processed = 0;
    const char* pos = macString;

    for (; *pos && bytes_processed < 6; pos += 3) {
        unsigned int byte = 0;
        if (sscanf(pos, "%2X", &byte) != 1) {
            fprintf(stderr, "Error parsing MAC address at %d\n" ,bytes_processed);
            return 0;
        }
        macAddr = (macAddr << 8) | (byte & 0xFF);
        bytes_processed++;

        if (bytes_processed < 6 && *pos != '\0' && *(pos + 2) != ':') {
            fprintf(stderr, "MAC address format error at %d\n", bytes_processed);
            return 0;
        }
    }

    if (bytes_processed != 6 || *pos != '\0') {
        fprintf(stderr, "Invalid MAC address length\n");
        return 0;
    }

    return macAddr;
}

//-----------------------------------------------------------------------------
// Main function to test the code.

int main (int argc, char *argv[]) {

    int opt;
    char *ip = NULL;
    struct sockaddr_in sockAddr = {0};
    unsigned long macAddress = 0x12345678AB;
    mcAddress(macAddress);
    
    while ((opt = getopt(argc, argv, "i:m:lsrh")) != -1) {
        switch (opt) {
            case 'i':  // handles ip addresses
                if (!inet_aton(optarg, &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
                    perror("Invalid IP Address");
                    return EXIT_FAILURE;
                }
                break;  
            case 'm':
                macAddress = parse_mcAddress(optarg); // handles mac addresses
                mcAddress(macAddress);
            
                break;
            case 'l':
            // list connected hosts
                break;
            case 's':
            // sends data
                break;
            case 'r':
            // receives data
                break;
            case 'h':
            // displays help
            display_help();
                exit(EXIT_SUCCESS);
            default:
                display_help();
                exit(EXIT_FAILURE);
        }   
    }

    int sockfd;                     
    struct icmphdr icmp_hdr;
    struct iphdr ip_hdr;

    // craft icmp echo request packet
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
    if (sockfd < 0) {
        perror("Socket error");
        exit(-1);
    }

    // set icmp header fields: type, code, checksum, id, sequence number
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.code = 0;
    icmp_hdr. checksum = 0;
    // htons(Host to Network Short) ensures proper network byte order
    icmp_hdr.un.echo.id = htons(getpid());  
    icmp_hdr.un.echo.sequence = htons(1);

    return 0;

    // arguments for cli

    //  calculate the checksum of the icmp header

    // send packet
    // sendto(socket, packet, packet_size, 0, (struct socketaddr*)&target_addr, sizeof(target_addr));

    // listen for reply

    // recvfrom(socket, buffer, buffer_size, 0, NULL, NULL);

    // check if the reply is an icmp echo reply
}



