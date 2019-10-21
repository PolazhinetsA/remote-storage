#ifndef SENDRECV_H
#define SENDRECV_H

void send_file(int sock, int file);
void recv_file(int sock, int file);

void send_msg(int sock, void *msg, uint32_t len);
void send_str(int sock, char *msg);
void recv_msg(int sock, char *msg);

#endif
