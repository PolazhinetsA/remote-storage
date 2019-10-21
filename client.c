#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "commondef.h"
#include "sendrecv.h"

int initconn(char *, char *);

int  login(int);
void mainloop(int);

int main(int argc, char **argv)
{
    mkdir("downloads", 0700);

    int sock = initconn(argv[1], argv[2]);

    if (-1 == sock) return -1;

    if (0 != login(sock)) return 1;

    mainloop(sock);

    close(sock);

    return 0;
}

int initconn(char *ipaddr, char *portno)
{
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(atoi(portno));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    return 0 == connect(sock, (struct sockaddr *)&addr, sizeof(addr))
           ? sock
           : -1;
}

int login(int sock)
{
    unsigned char treq;
    char opt[0x10];
    user_t info;

    printf("login or register? (l/r) ");
    scanf("%16s", opt);
    treq = opt[0] == 'l' ? LOGIN : REGISTER;

    printf("username: ");
    scanf("%s", info.username);

    printf("password: ");
    scanf("%s", info.password);

    write(sock, &treq, 1);
    write(sock, &info, sizeof(user_t));

    char resp[0x100];
    recv_msg(sock, resp);
    puts(resp);
    return strcmp(resp, "ok");
}

void mainloop(int sock)
{
    for(char opt[0x10];
        printf("list, upload, download or quit? (l/u/d/q) "),
        scanf("%16s", opt),
        opt[0] != 'q'; )
    {
        unsigned char treq;

        switch (opt[0])
        {
        case 'l':
        {
            treq = LIST;
            write(sock, &treq, 1);

            for(char msg[0x100];
                recv_msg(sock, msg),
                strcmp(msg, "//");
                puts(msg));

            break;
        }
        case 'u':
        {
            treq = UPLOAD;
            write(sock, &treq, 1);

            char path[0x100];
            printf("path: ");
            scanf("%s", path);

            char *filename = strrchr(path, '/');
            send_str(sock, filename ? filename+1 : path);

            int file = open(path, O_RDONLY);
            send_file(sock, file);
            close(file);

            break;
        }
        case 'd':
        {
            treq = DOWNLOAD;
            write(sock, &treq, 1);

            char path[0x100], filename[0x100];
            printf("filename: ");
            scanf("%s", filename);
            send_str(sock, filename);

            sprintf(path, "downloads/%s", filename);
            int file = open(path, O_CREAT | O_WRONLY, 0600);
            recv_file(sock, file);
            close(file);

            break;
        }
        }
    }
}
