#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <stdint.h>

typedef struct {
    char username[0x20];
    char password[0x20];
} user_t;

enum { REGISTER, LOGIN, NTREQ1, LIST, UPLOAD, DOWNLOAD, NTREQ2 };

#define PERROR(err)             \
if (err)                        \
{                               \
    char _errorstring[0x100];   \
    perror(_errorstring);       \
    fputs(_errorstring, stderr);\
}

#endif
