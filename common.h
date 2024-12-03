#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <memory>
#ifdef WIN32
#ifdef _MSC_VER
#ifndef UNICODE
#define UNICODE 1
#endif
// link with Ws2_32.lib
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <memoryapi.h>
#include <iphlpapi.h>

// Need to link with Iphlpapi.lib
#pragma comment(lib, "iphlpapi.lib")
// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define FOPEN fopen
#define FSEEK _fseeki64
#define FTELL _ftelli64
#define SNPRINTF _snprintf
#define SOCK SOCKET
#define SHUT_RETR SD_BOTH
#define SOCKCLOSE closesocket
#define NET_EINPROGRESS WSAEWOULDBLOCK
#define NET_STRERROR net_strerror
#define NET_ERRNO WSAGetLastError()
#define SLEEP_MS(x) Sleep(x)
#endif
#ifdef __MINGW32__
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <memoryapi.h>
#include <iphlpapi.h>
#include <tcpestats.h>
#include <sys/stat.h>
#define FOPEN fopen64
#define FSEEK fseeko64
#define FTELL ftello64
#define SNPRINTF snprintf
#define SOCK int
#define SHUT_RETR SD_BOTH
#define SOCKCLOSE closesocket
#define NET_EINPROGRESS WSAEWOULDBLOCK
#define NET_STRERROR net_strerror
#define NET_ERRNO WSAGetLastError()
#define SLEEP_MS(x) Sleep(x) // usleep(x*1000)
#endif
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <features.h>
#include <linux/version.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <semaphore.h>
#include <sys/shm.h>
#include <utility>

#define FOPEN fopen64
#define FSEEK fseeko64
#define FTELL ftello64
#define SNPRINTF snprintf
#define SOCK int
#define INVALID_SOCKET -1
#define SHUT_RETR SHUT_RDWR
#define SOCKCLOSE close
#define NET_EINPROGRESS EINPROGRESS
#define NET_STRERROR strerror
#define NET_ERRNO errno
#define SLEEP_MS(x) usleep(x * 1000)
#endif
#endif // COMMON_H
