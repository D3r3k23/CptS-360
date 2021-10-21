#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

void send_file(int sock, const char* pathname);
void recv_file(int sock, const char* pathname);

#endif // FILE_TRANSFER_H
