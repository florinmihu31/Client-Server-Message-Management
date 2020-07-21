#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <iostream>
#include <vector>

#define BUFLEN 1600
#define MAX_CLIENTS 10 // Numarul maxim de clienti in asteptare
#define SUBSCRIBE 1
#define UNSUBSCRIBE 2

// Structura pentru mesajele catre server
struct __attribute__((packed)) message_to_server {
    int action;
    char topic[50];
    int sf;
};

// Structura pentru mesajele catre client
struct __attribute__((packed)) message_to_client {
    char ip[16];
    int port_no;
    char topic[50];
    uint8_t data_type;
    char content[1500];
};

// Structura pentru topic
struct __attribute__((packed)) topic {
    char name[50];
    int sf;
};

// Structura pentru client
struct client {
    int sockfd;
    char id[11];
    bool is_online;
    std::vector<topic> subscribed_topics;
    std::vector<message_to_client> messages;
};

#endif