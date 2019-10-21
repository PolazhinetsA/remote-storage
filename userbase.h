#ifndef USERBASE_H
#define USERBASE_H

void loadusers();

int userid(char *username);
int checkpw(int uid, char *password);
int adduser(user_t *user);

#endif
