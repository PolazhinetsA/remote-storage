#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "commondef.h"
#include "userbase.h"

const user_t user0 = {"zero", "zero"};

int      fusers;
size_t szfusers;
user_t   *users;
int      nusers;

void loadusers()
{
    struct stat st;

    fusers = open("users", O_CREAT | O_RDWR, 0600);
    fstat(fusers, &st);
    if (st.st_size == 0)
    {
        mkdir("0", 0700);
        uint32_t nusers = 0;
        write(fusers, &user0, sizeof(user_t));
        fstat(fusers, &st);
    }
    szfusers = st.st_size;
    nusers = szfusers / sizeof(user_t);

    users = mmap(NULL, szfusers, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE, fusers, 0);
}

int adduser(user_t *puser)
{
    lseek(fusers, 0, SEEK_END);
    write(fusers, puser, sizeof(user_t));

    size_t szfusers_ = szfusers + sizeof(user_t);
    users = mremap(users, szfusers, szfusers_, MREMAP_MAYMOVE);

    char dirname[0x10];
    sprintf(dirname, "%u", nusers);
    mkdir(dirname, 0700);

    return nusers++;
}

int userid(char *username)
{
    for (int i = 0; i < nusers; ++i)
    {
        if (!strcmp(users[i].username, username))
            return i;
    }
    return -1;
}

int checkpw(int uid, char *password)
{
    return 0 <= uid && uid < nusers
        && !strcmp(users[uid].password, password);
}
