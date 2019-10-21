#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#include "commondef.h"
#include "sendrecv.h"

void send_file(int sock, int file)
{
    struct stat st;
    fstat(file, &st);

    uint32_t szfile = st.st_size;
    write(sock, &szfile, sizeof(uint32_t));

    sendfile(sock, file, NULL, szfile);
}

void recv_file(int sock, int file)
{
    uint32_t szfile;
    read(sock, &szfile, sizeof(uint32_t));

    char buf[0x1000];
    ssize_t nrecv, nread;

    for (nrecv = 0; nrecv + sizeof(buf) <= szfile; )
    {
        nread = read(sock, buf, sizeof(buf));
        write(file, buf, nread);
        nrecv += nread;
    }
    for (; nrecv < szfile; )
    {
        nread = read(sock, buf, szfile - nrecv);
        write(file, buf, nread);
        nrecv += nread;
    }
}

void send_msg(int sock, void *msg, uint32_t len)
{
    write(sock, &len, sizeof(uint32_t));
    write(sock, msg, len);
}

void send_str(int sock, char *msg)
{
    send_msg(sock, msg, strlen(msg)+1);
}

void recv_msg(int sock, char *msg)
{
    uint32_t len;

    read(sock, &len, sizeof(uint32_t));
    read(sock, msg, len);
}
