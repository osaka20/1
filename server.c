#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(void) {
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 1) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Serveur en attente sur le port %d...\n", PORT);
    client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        close(listen_fd);
        return 1;
    }

    printf("Client connecté.\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_fd, &readfds);

        int maxfd = (STDIN_FILENO > client_fd) ? STDIN_FILENO : client_fd;

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
                if (send(client_fd, buf, len, 0) < 0) {
                    perror("send");
                    break;
                }
            }
        }

        if (FD_ISSET(client_fd, &readfds)) {
            ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (n == 0) {
                printf("Client déconnecté.\n");
                break;
            } else if (n < 0) {
                perror("recv");
                break;
            }
            buf[n] = '\0';
            printf("Client: %s", buf);
            fflush(stdout);
        }
    }

    close(client_fd);
    close(listen_fd);
    printf("Serveur fermé.\n");
    return 0;
}
