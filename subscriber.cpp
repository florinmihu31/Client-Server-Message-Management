#include "helpers.h"

int main(int argc, char *argv[]) {
    // Buffer in care primim mesajele
    char buffer[BUFLEN];

    // Verificam numarul de argumente
    if (argc != 4) {
        perror("Nu s-a introdus numarul de argumente corect\n");
        exit(EXIT_FAILURE);
    }

    // Deschidem socketul
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Eroare la socket\n");
        exit(EXIT_FAILURE);
    }

    // Numarul portului
    int port_no = atoi(argv[3]);

    if (port_no == 0) {
        perror("Eroare la numarul portului\n");
        exit(EXIT_FAILURE);
    }

    // Adaugam informatii despre server
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);

    // Verificam IP-ul
    int ip = inet_aton(argv[2], &serv_addr.sin_addr);

    if (ip == 0) {
        perror("Eroare la IP\n");
        exit(EXIT_FAILURE);
    }

    // Conectare la server
    int c = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (c < 0) {
        perror("Eroare la conexiune\n");
        exit(EXIT_FAILURE);
    }

    // Trimiterea id-ului
    int s = send(sockfd, argv[1], strlen(argv[1]) + 1, 0);

    if (s < 0) {
        perror("Eroare la trimitere\n");
        exit(EXIT_FAILURE);
    }

    // Dezactivarea algoritmului Nagle
    int flag = 1;
    int disable_nagle = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                                   (char *) &flag, sizeof(int));

    if (disable_nagle < 0) {
        perror("Eroare la dezactivarea algoritmului Nagle\n");
        exit(EXIT_FAILURE);
    }

    // Multimea file descriptorilor cititi
    fd_set read_fds;
    // Multime auxiliara
    fd_set tmp_fds;

    // Initializam multimile de descriptori
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Adaugam in multimile de descriptori socketul si stdin
    FD_SET(0, &read_fds);
    FD_SET(sockfd, &read_fds);

    while (true) {
        tmp_fds = read_fds;

        // Selectam din multimea de descriptori
        s = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);

        if (s < 0) {
            perror("Eroare la select\n");
            exit(EXIT_FAILURE);
        }

        // Daca mesajul este de la tastatura
        if (FD_ISSET(0, &tmp_fds)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            // Daca s-a primit o comanda de iesire, oprim bucla
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }

            /*
             * Parsam mesajul pe care il primim de la tastatura si il stocam
             * intr-o structura message_to_server
             */
            message_to_server message;
            // Vectori auxiliari pentru citire
            char command[20], topic[50], sf[5];
            // Pozitia pointerului
            int current_pos = 0;

            // Stocam comanda
            strncpy(command, buffer, 20);

            for (int i = 0; i < strlen(command); i++) {
                if (command[i] == ' ') {
                    command[i] = '\0';
                    current_pos += i;
                    break;
                }
            }

            // Comanda primita este de subscribe
            if (strcmp(command, "subscribe") == 0) {
                bool is_valid = true;

                if (buffer[current_pos] != ' ') {
                    std::cout << "Comanda nu are spatiu" << std::endl;
                    continue;
                }

                // Setam actiunea ca fiind de tip SUBSCRIBE
                message.action = SUBSCRIBE;
                // Sarim peste spatiu
                current_pos++;

                // Copiem topicul
                strncpy(topic, buffer + current_pos, 50);

                // Parcurgem litera cu litera topicul si verificam daca e valid
                for (int i = 0; i < 50; i++) {
                    if (topic[i] == '\n' || topic[i] == '\0') {
                        std::cout << "Nu a fost introdus un topic corect"
                                  << std::endl;
                        is_valid = false;
                        break;
                    } else if (topic[i] == ' ') {
                        topic[i] = '\0';
                        current_pos += i;
                        break;
                    }
                }

                if (!is_valid) {
                    continue;
                }

                // Copiem topicul
                strcpy(message.topic, topic);

                // Sarim peste spatiu
                current_pos++;
                strncpy(sf, buffer + current_pos, 5);

                if (sf[0] != '0' && sf[0] != '1') {
                    std::cout << "Nu a fost introdus un sf valid"
                              << std::endl;
                    continue;
                } else if (sf[0] == '0' || sf[0] == '1') {
                    message.sf = sf[0] - '0';
                }

                if (sf[1] != '\n') {
                    std::cout << "Comanda are prea multe argumente/argumente gresite"
                              << std::endl;
                    continue;
                }

                // Punem in buffer mesajul
                memcpy(buffer, &message, sizeof(struct message_to_server));

                // Trimitem mesajul catre server
                s = send(sockfd, buffer, BUFLEN, 0);

                if (s < 0) {
                    perror("Eroare la trimiterea mesajului de subscribe\n");
                    continue;
                }

                std::cout << "Utilizatorul s-a abonat la topicul "
                          << message.topic << ", SF: " << message.sf
                          << std::endl;
            } else if (strcmp(command, "unsubscribe") == 0) {
                // Comanda de unsubscribe
                bool is_valid = true;

                if (buffer[current_pos] != ' ') {
                    std::cout << "Comanda nu are spatiu" << std::endl;
                    continue;
                }

                // Setam actiunea ca fiind de tip UNSUBSCRIBE
                message.action = UNSUBSCRIBE;
                // Sarim peste spatiu
                current_pos++;

                // Copiem topicul
                strncpy(topic, buffer + current_pos, 50);

                // Parcurgem litera cu litera topicul si verificam daca e valid
                for (int i = 0; i < 50; i++) {
                    if (topic[i] == ' ' || topic[i] == '\0') {
                        std::cout << "Nu a fost introdus un topic corect"
                                  << std::endl;
                        is_valid = false;
                        break;
                    } else if (topic[i] == '\n') {
                        topic[i] = '\0';
                        current_pos += i;
                        break;
                    }
                }

                if (!is_valid) {
                    continue;
                }

                // Copiem topicul
                strcpy(message.topic, topic);

                // Punem mesajul in buffer
                memcpy(buffer, &message, sizeof(struct message_to_server));

                // Trimitem mesajul serverului
                s = send(sockfd, buffer, BUFLEN, 0);

                if (s < 0) {
                    perror("Eroare la trimiterea mesajului de unsubscribe\n");
                    continue;
                }

                std::cout << "Utilizatorul s-a dezabonat de la topicul "
                          << message.topic << std::endl;
            } else {
                perror("Comanda inexistenta\n");
                continue;
            }
        }

        // Primire mesaj de la server
        if (FD_ISSET(sockfd, &tmp_fds)) {
            // Structura in care vom primi mesajul
            message_to_client* message;

            memset(buffer, 0, BUFLEN);

            // Receptarea mesajului de la server
            int received = recv(sockfd, buffer,
                                sizeof(struct message_to_client), 0);

            if (received < 0) {
                perror("Eroare la primirea mesajului de la server.\n");
                continue;
            }

            // Castam mesajul la structura corespunzatoare
            message = (message_to_client *) buffer;

            // Daca nu se mai primeste nimic, inchidem conexiunea
            if (received == 0) {
                break;
            }

            char type[11];
            bool is_valid = true;

            // Verificam tipul de date
            switch (message->data_type) {
                case 0:
                    strcpy(type, "INT");
                    break;

                case 1:
                    strcpy(type, "SHORT_REAL");
                    break;

                case 2:
                    strcpy(type, "FLOAT");
                    break;

                case 3:
                    strcpy(type, "STRING");
                    break;

                default:
                    perror("Eroare la tipul mesajului\n");
                    is_valid = false;
            }

            // Daca nu este valid, continuam bucla
            if (!is_valid) {
                continue;
            }

            // Afisam informatiile corespunzatoare
            std::cout << message->ip << ":" << message->port_no << " - "
                      << message->topic << " - "  << type << " - "
                      << message->content << std::endl;
        }
    }

    // Inchidem socketii
    close(sockfd);
    close(0);

    return 0;
}
