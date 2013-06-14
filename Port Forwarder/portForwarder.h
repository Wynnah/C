#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>

#define _GNU_SOURCE	
#define BUF_SIZE    1024        //Buffer length
#define TRUE    1
#define FALSE    0
#define MAXLISTENERS 5


#define SHUT_FD1 do {                                \
                     if (fd1 >= 0) {                 \
                         shutdown(fd1, SHUT_RDWR);   \
                         close(fd1);                 \
                         fd1 = -1;                   \
                     }                               \
                 } while (0)

#define SHUT_FD2 do {                                \
                     if (fd2 >= 0) {                 \
                         shutdown(fd2, SHUT_RDWR);   \
                         close(fd2);                 \
                         fd2 = -1;                   \
                     }                               \
                 } while (0)
                 
#define max(x,y) ((x) > (y) ? (x) : (y))

/* struct with c members x, y */
typedef struct {
  int    curr_sd;
  char* clientAddr;
  int port;
  int forward_sd;
  char* forward_ip;
  int forward_port;
} connInfo;

typedef struct{
    int listen_port;
    int forward_port;
    char* forward_ip;
} forwardInfo;

static int connect_socket(int connect_port, char *address);
void* outputMsg (void* msg);
char* substring(const char* str, size_t begin, size_t len);
forwardInfo readConfig(int currChild);
static void abortSystem(const char* message);
void child(int currChild);
int maxChild();
