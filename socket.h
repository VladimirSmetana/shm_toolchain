#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

class Socket
{
public:
    int ret;
    int newSD = -1;
    int serverSd = -1;
    int send_ofs = 0;

    void init_socket(int p_number);
    void uninit_socket();
    int send_data(int p_number, uint8_t *reader, size_t send_size);
};