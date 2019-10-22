#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "commondef.h"
#include "userbase.h"
#include "sendrecv.h"

int  initsock(int);
void mainloop(int);

int main(int argc, char **argv)
{
    loadusers();

    int sock;
    
    sock = initsock(atoi(argv[1]));
    mainloop(sock);
    close(sock);
}

int initsock(int portno)
{
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 5);

    return sock;
}

void *process(void *);

void mainloop(int sock)
{
    while (1)
    {
        long _sock;
        struct sockaddr_in _addr;
        socklen_t _szaddr;
        char str_addr[INET_ADDRSTRLEN];

        _sock = accept(sock, (struct sockaddr *)&_addr, &_szaddr);

        inet_ntop(AF_INET, &_addr.sin_addr, str_addr, INET_ADDRSTRLEN);
        printf("connection accepted from %s\n", str_addr);

        pthread_t tid;
        pthread_create(&tid, NULL, process, (void *)_sock);
        pthread_detach(tid);
    }
}

int s_register  (int);
int s_login     (int);
int s_list      (int, char *);
int s_upload    (int, char *);
int s_download  (int, char *);

void *process(void *_sock)
{
    int sock, uid;
    unsigned char treq;
    sock = (long )_sock;

    read(sock, &treq, 1);
    switch (treq)
    {
        case REGISTER:  uid = s_register(sock); break;
        case LOGIN:     uid = s_login(sock);    break;
        default:        goto end;
    }
    if (uid == -1) goto end;

    printf("user %d logged in\n", uid);

    char dirname[0x100];
    sprintf(dirname, "%d", uid);

    while (read(sock, &treq, 1))
    {
        int rval;
        switch (treq)
        {
            case LIST:      rval = s_list(sock, dirname);       break;
            case UPLOAD:    rval = s_upload(sock, dirname);     break;
            case DOWNLOAD:  rval = s_download(sock, dirname);   break;
            default:        goto end;
        }
        if (rval == -1) break;
    }

    printf("user %d disconnected\n", uid);
    
    end:
    close(sock);
}

int s_register(int sock)
{
    user_t user;
    read(sock, &user, sizeof(user_t));

    if (-1 != userid(user.username))
    {
        send_str(sock, "user exists");
        return -1;
    }
    send_str(sock, "ok");
    return adduser(&user);
}

int s_login(int sock)
{
    user_t user;
    int uid;

    read(sock, &user, sizeof(user_t));
    uid = userid(user.username);

    if (uid == -1)
    {
        send_str(sock, "no such user");
        return -1;
    }
    if (!checkpw(uid, user.password))
    {
        send_str(sock, "wrong password");
        return -1;
    }

    send_str(sock, "ok");
    return uid;
}

int s_list(int sock, char *dirname)
{
    DIR *dir = opendir(dirname);

    for(struct dirent *ent;
        ent = readdir(dir);
        send_str(sock, ent->d_name));

    send_str(sock, "//");

    closedir(dir);

    return 0;
}

int s_upload(int sock, char *dirname)
{
    char path[0x100], filename[0x100];

    recv_msg(sock, filename);
    sprintf(path, "%s/%s", dirname, filename);

    int file;

    file = open(path, O_CREAT | O_WRONLY, 0600);
    recv_file(sock, file);
    close(file);

    return 0;
}

int s_download(int sock, char *dirname)
{
    struct stat st;
    char path[0x100], filename[0x100];

    recv_msg(sock, filename);
    sprintf(path, "%s/%s", dirname, filename);

    int file = open(path, O_RDONLY);
    send_file(sock, file);
    close(file);

    return 0;
}
