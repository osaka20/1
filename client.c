#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

static void print_wsa_error(const char *where) {
    int err = WSAGetLastError();
    fprintf(stderr, "%s failed (WSA error %d)\n", where, err);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_SERVEUR> <PORT>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        print_wsa_error("WSAStartup");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        print_wsa_error("socket");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Adresse IP invalide: %s\n", server_ip);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_wsa_error("connect");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connecte au serveur %s:%d\n", server_ip, port);
    printf("Tapez votre message et appuyez sur Entree.\n");
    printf("> ");
    fflush(stdout);

    char recv_buf[BUF_SIZE];
    char input_buf[BUF_SIZE];
    size_t input_len = 0;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        int ready = select(0, &readfds, NULL, NULL, &tv);
        if (ready == SOCKET_ERROR) {
            print_wsa_error("select");
            break;
        }

        if (FD_ISSET(sock, &readfds)) {
            int n = recv(sock, recv_buf, (int)sizeof(recv_buf) - 1, 0);
            if (n == 0) {
                printf("\nServeur deconnecte.\n");
                break;
            }
            if (n == SOCKET_ERROR) {
                print_wsa_error("recv");
                break;
            }
            recv_buf[n] = '\0';
            printf("\nServeur: %s", recv_buf);
            if (input_len > 0) {
                printf("> %.*s", (int)input_len, input_buf);
            } else {
                printf("> ");
            }
            fflush(stdout);
        }

        while (_kbhit()) {
            int ch = _getch();
            if (ch == '\r' || ch == '\n') {
                if (input_len > 0) {
                    input_buf[input_len++] = '\n';
                    if (send(sock, input_buf, (int)input_len, 0) == SOCKET_ERROR) {
                        print_wsa_error("send");
                        goto cleanup;
                    }
                }
                input_len = 0;
                printf("\n> ");
                fflush(stdout);
            } else if (ch == '\b') {
                if (input_len > 0) {
                    input_len--;
                    printf("\b \b");
                    fflush(stdout);
                }
            } else if (ch >= 32 && ch <= 126) {
                if (input_len < sizeof(input_buf) - 2) {
                    input_buf[input_len++] = (char)ch;
                    putchar(ch);
                    fflush(stdout);
                }
            }
        }
    }

cleanup:
    closesocket(sock);
    WSACleanup();
    printf("Client ferme.\n");
    return 0;
}
