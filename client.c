#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_SERVEUR> <PORT>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("Connecté au serveur %s:%d\n", server_ip, port);

    char buf[BUF_SIZE];

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock_fd, &readfds);

        int maxfd = (STDIN_FILENO > sock_fd) ? STDIN_FILENO : sock_fd;

        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                printf("Fin de stdin.\n");
                break;
            }
            size_t len = strlen(buf);
            if (len > 0) {
                if (send(sock_fd, buf, len, 0) < 0) {
                    perror("send");
                    break;
                }
            }
        }

        if (FD_ISSET(sock_fd, &readfds)) {
            ssize_t n = recv(sock_fd, buf, sizeof(buf) - 1, 0);
            if (n == 0) {
                printf("Serveur déconnecté.\n");
                break;
            } else if (n < 0) {
                perror("recv");
                break;
            }
            buf[n] = '\0';
            printf("Serveur: %s", buf);
            fflush(stdout);
        }
    }

    close(sock_fd);
    printf("Client fermé.\n");
    return 0;
}
