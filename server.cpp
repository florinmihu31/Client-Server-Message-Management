#include "helpers.h"

int main(int argc, char* argv[]) {
    // Buffer in care primim mesajele
    char buffer[BUFLEN];
    // Structuri pentru server, client TCP, client UDP
    struct sockaddr_in serv_addr, cli_tcp_addr, cli_udp_addr;
    // Noul socket citit
    int new_sockfd;
    // Lungimile socketilor ptr TCP si UDP
    socklen_t cli_tcp_len, cli_udp_len;

    // Multimea file descriptorilor cititi
    fd_set read_fds;
    // Multime auxiliara
    fd_set tmp_fds;

    // Vector in care sunt retinuti clientii
    std::vector<client> clients;

    // Verificam numarul de argumente
    if (argc != 2) {
        perror("Nu s-a introdus numarul de argumente corect\n");
        exit(EXIT_FAILURE);
    }

    // Initializam multimile de descriptori
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Deschidem socketul TCP
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_sockfd < 0) {
        perror("Eroare la socket-ul TCP\n");
        exit(EXIT_FAILURE);
    }

    // Deschidem socketul UDP
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_sockfd < 0) {
        perror("Eroare la socket-ul UDP\n");
        exit(EXIT_FAILURE);
    }

    // Numarul portului
    int port_no = atoi(argv[1]);

    if (port_no == 0) {
        perror("Eroare la numarul portului\n");
        exit(EXIT_FAILURE);
    }

    // Adaugam informatii despre server
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Adaugam informatii despre client TCP
    cli_tcp_addr.sin_family = AF_INET;
    cli_tcp_addr.sin_port = htons(port_no);
    cli_tcp_addr.sin_addr.s_addr = INADDR_ANY;

    // Adaugam informatii despre UDP
    cli_udp_addr.sin_family = AF_INET;
    cli_udp_addr.sin_port = htons(port_no);
    cli_udp_addr.sin_addr.s_addr = INADDR_ANY;

    // Realizam legatura intre TCP si server
    int b = bind(tcp_sockfd, (struct sockaddr*) &cli_tcp_addr,
                 sizeof(struct sockaddr));

    if (b < 0) {
        perror("Eroare la bind TCP\n");
        exit(EXIT_FAILURE);
    }

    // Realizam legatura intre UDP si server
    b = bind(udp_sockfd, (struct sockaddr*) &cli_udp_addr,
             sizeof(struct sockaddr));

    if (b < 0) {
        perror("Eroare la bind UDP\n");
        exit(EXIT_FAILURE);
    }

    // Ascultam pe socketul de TCP
    int l = listen(tcp_sockfd, MAX_CLIENTS);

    if (l < 0) {
        perror("Eroare la listen\n");
        exit(EXIT_FAILURE);
    }

    // Adaugam in multimile de descriptori socketii de TCP, UDP si stdin
    FD_SET(0, &read_fds);
    FD_SET(tcp_sockfd, &read_fds);
    FD_SET(udp_sockfd, &read_fds);
    int fdmax = tcp_sockfd;

    while (true) {
        // Conditie de iesire din bucla
        bool break_loop = false;
        tmp_fds = read_fds;

        // Selectam din multimea de descriptori
        int s = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

        if (s < 0) {
            perror("Eroare la select\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                // Comanda de la tastatura
                if (i == 0) {
                    fgets(buffer, BUFLEN, stdin);

                    // Comanda de iesire
                    if (strncmp(buffer, "exit", 4) == 0) {
                        // Activam conditia de iesire din bucla
                        break_loop = true;
                        break;
                    } else {
                        std::cout << "Comanda inexistenta" << std::endl;
                    }
                } else if (i == tcp_sockfd) {
                    // Comanda de conectare a unui client TCP
                    cli_tcp_len = sizeof(cli_tcp_addr);
                    // Acceptam conexiunea pe noul socket
                    new_sockfd = accept(tcp_sockfd,
                                        (struct sockaddr *) &cli_tcp_addr, &cli_tcp_len);

                    if (new_sockfd < 0) {
                        perror("Eroare la acceptul new_sockfd");
                        exit(EXIT_FAILURE);
                    }

                    // Dezactivam algoritmul Nagle
                    int flag = 1;
                    int disable_nagle = setsockopt(new_sockfd, IPPROTO_TCP,
                                                   TCP_NODELAY, (char *) &flag, sizeof(int));

                    if (disable_nagle < 0) {
                        perror("Eroare la dezactivarea algoritmului Nagle\n");
                        exit(EXIT_FAILURE);
                    }

                    // Adaugam noul socket in multime
                    FD_SET(new_sockfd, &read_fds);

                    // Actualizam maximul
                    if (new_sockfd > fdmax) {
                        fdmax = new_sockfd;
                    }

                    // Receptionam numele clientului
                    int r = recv(new_sockfd, buffer, BUFLEN, 0);

                    if (r < 0) {
                        perror("Eroare la primirea mesajului\n");
                        break;
                    }

                    // Boolean ce determina daca utilizatorul a fost gasit
                    bool found = false;
                    // Boolean ce determina daca utilizatorul este conectat
                    bool is_connected = false;

                    for (int j = 0; j < clients.size(); j++) {
                        // Daca gasim id-ul in multimea clientilor
                        if (strcmp(buffer, clients[j].id) == 0) {
                            // Daca utilizatorul e online, afisam ca e deja conectat
                            if (clients[j].is_online) {
                                found = true;
                                is_connected = true;
                                perror("Utilizatorul e deja conectat.\n");
                                break;
                            } else {
                                /*
                                 * Daca utilizatorul nu e online, parcurgem lista
                                 * de mesaje offline si le trimitem catre client
                                 */
                                found = true;
                                clients[j].is_online = true;
                                clients[j].sockfd = new_sockfd;

                                for (int k = 0; k < clients[j].messages.size(); k++) {
                                    s = send(clients[j].sockfd,
                                             &clients[j].messages[k],
                                             sizeof(clients[j].messages[k]), 0);

                                    if (s < 0) {
                                        perror("Eroare la transmiterea mesajelor offline\n");
                                    }
                                }
                                // Stergem lista de mesaje
                                clients[j].messages.clear();
                                break;
                            }
                        }
                    }

                    // Daca nu am gasit clientul, il adaugam in vector
                    if (!found) {
                        client new_client;
                        new_client.sockfd = new_sockfd;
                        new_client.is_online = true;
                        strcpy(new_client.id, buffer);
                        clients.push_back(new_client);
                    }

                    // Daca utilizatorul era deja online, ii inchidem conexiunea
                    if (is_connected) {
                        close(new_sockfd);
                        FD_CLR(new_sockfd, &read_fds);
                        continue;
                    }

                    std::cout << "New client " << buffer << " connected from "
                              << inet_ntoa(cli_tcp_addr.sin_addr) << ":"
                              << ntohs(cli_tcp_addr.sin_port) << std::endl;
                } else if (i == udp_sockfd) {
                    // Comanda de primire de la un client UDP
                    memset(buffer, 0, BUFLEN);

                    // Primeste mesajul de la clientul UDP
                    cli_udp_len = sizeof(cli_udp_addr);
                    int received = recvfrom(udp_sockfd, buffer, BUFLEN, 0,
                                            (struct sockaddr*) &cli_udp_addr, &cli_udp_len);

                    if (received < 0) {
                        perror("Eroare la primirea mesajului (client UDP)\n");
                        exit(EXIT_FAILURE);
                    }

                    // Mesajul pe care il vom trimite catre client
                    message_to_client msg_to_send;

                    // Adaugam IP-ul si portul
                    strcpy(msg_to_send.ip, inet_ntoa(cli_udp_addr.sin_addr));
                    msg_to_send.port_no = ntohs(cli_udp_addr.sin_port);

                    // Primii 50 de octeti din buffer vor fi topicul
                    memcpy(&msg_to_send.topic, buffer, 50);
                    // Stocam tipul de date
                    memcpy(&msg_to_send.data_type, buffer + 50,
                           sizeof(uint8_t));

                    // In functie de tipul de date completam restul campurilor
                    if (msg_to_send.data_type == 0) {
                        uint8_t sign;
                        uint32_t payload;

                        memcpy(&sign, buffer + 50 + sizeof(uint8_t),
                               sizeof(uint8_t));
                        memcpy(&payload, buffer + 50 + sizeof(uint8_t)
                                + sizeof(uint8_t), sizeof(uint32_t));

                        payload = ntohl(payload);

                        if (sign == 1) {
                            payload = -payload;
                        }

                        sprintf(msg_to_send.content, "%d", payload);
                    } else if (msg_to_send.data_type == 1) {
                        uint16_t payload;

                        memcpy(&payload, buffer + 50 + sizeof(uint8_t),
                               sizeof(uint16_t));

                        float result = ntohs(payload) / 100.0;

                        sprintf(msg_to_send.content, "%f", result);
                    } else if (msg_to_send.data_type == 2) {
                        uint32_t number;
                        uint8_t power, sign;
                        float payload;

                        memcpy(&sign, buffer + 50 + sizeof(uint8_t),
                               sizeof(uint8_t));
                        memcpy(&number, buffer + 50 + sizeof(uint8_t)
                                        + sizeof(uint8_t), sizeof(uint32_t));
                        memcpy(&power, buffer + 50 + sizeof(uint8_t)
                                       + sizeof(uint8_t) + sizeof(uint32_t),
                               sizeof(uint8_t));

                        payload = ntohl(number);

                        for (int j = 0; j < power; j++) {
                            payload /= 10.0;
                        }

                        if (sign == 1) {
                            payload = -payload;
                        }

                        sprintf(msg_to_send.content, "%f", payload);
                    } else if (msg_to_send.data_type == 3) {
                        strcpy(msg_to_send.content, buffer + 50
                                + sizeof(uint8_t));
                    } else {
                        perror("Eroare la tipul mesajului\n");
                        continue;
                    }

                    /*
                     * Parcurgem listele tuturor clientilor, iar daca utilizatorul
                     * curent este abonat si online, ii trimitem mesajul, altfel,
                     * il adaugam in lista personala de mesaje
                     */
                    for (int k = 0; k < clients.size(); k++) {
                        for (int j = 0; j < clients[k].subscribed_topics.size();
                             j++) {
                            if (strcmp(msg_to_send.topic,
                                       clients[k].subscribed_topics[j].name) == 0) {
                                if (clients[k].is_online) {
                                    send(clients[k].sockfd, &msg_to_send,
                                         sizeof(msg_to_send), 0);
                                } else if (clients[k].subscribed_topics[j].sf == 1) {
                                    clients[k].messages.push_back(msg_to_send);
                                }
                            }
                        }
                    }
                } else {
                    // S-au primit comenzi de subscribe/unsubscribe
                    memset(buffer, 0, BUFLEN);

                    // Receptionam comanda
                    int received = recv(i, buffer, BUFLEN, 0);

                    if (received < 0) {
                        perror("Eroare la primirea mesajului (client TCP)\n");
                        exit(EXIT_FAILURE);
                    }

                    /*
                     * Daca nu se mai primeste nimic inseamna ca abonatul s-a
                     * deconectat, deci ii trecem statusul in offline si ii
                     * inchidem socketul
                    */
                    if (received == 0) {
                        for (int j = 0; j < clients.size(); j++) {
                            if (clients[j].sockfd == i) {
                                clients[j].is_online = false;
                                std::cout << "Client " << clients[j].id
                                          << " disconnected." << std::endl;
                                break;
                            }
                        }

                        close(i);
                        FD_CLR(i, &read_fds);
                    } else {
                        /*
                         * Facem cast mesajului primit in buffer intr-o structura
                         * message_to_server
                         */
                        message_to_server* message;
                        message = (message_to_server *) buffer;

                        if (message->action == SUBSCRIBE) {
                            /*
                             * Daca actiunea este de tip SUBSCRIBE, atunci gasim
                             * clientul curent si ii adaugam un nou topic si sf-ul
                             * aferent in lista de topicuri la care e abonat
                             */
                            for (int j = 0; j < clients.size(); j++) {
                                if (clients[j].sockfd == i) {
                                    topic new_topic;

                                    strcpy(new_topic.name, message->topic);
                                    new_topic.sf = message->sf;

                                    clients[j].subscribed_topics
                                            .push_back(new_topic);
                                }
                            }
                        } else if (message->action == UNSUBSCRIBE) {
                            /*
                             * Daca actiunea este de tip UNSUBSCRIBE, atunci gasim
                             * clientul curent, gasim topicul la care doreste sa
                             * se dezaboneze si il eliminam din vectorul de topicuri
                             * la care e abonat
                             */
                            for (int j = 0; j < clients.size(); j++) {
                                if (clients[j].sockfd == i) {
                                    for (auto k = clients[j].subscribed_topics.begin();
                                         k != clients[j].subscribed_topics.end();
                                         k++) {
                                        if (strcmp(message->topic, k->name) == 0) {
                                            clients[j].subscribed_topics.erase(k);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Oprim bucla daca este activata conditia break_loop
        if (break_loop) {
            break;
        }
    }

    // Inchidem socketii
    for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &read_fds)) {
            close(i);
            FD_CLR(i, &read_fds);
        }

        if (FD_ISSET(i, &tmp_fds)) {
            close(i);
            FD_CLR(i, &tmp_fds);
        }
    }

    close(tcp_sockfd);
    close(udp_sockfd);

    return 0;
}
