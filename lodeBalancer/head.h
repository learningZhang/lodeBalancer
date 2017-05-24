#include <iostream>
#include <string>
#include <map>
using namespace std;
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <event.h>
#include <json/json.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <list>
#include "mysql.h"

<<<<<<< HEAD
<<<<<<< Updated upstream
class CMysql
=======

#define THREAD_NUM 3
#define MAX 50 //µ¥Ïß³ÌÖĞÒ»¸öepoll×î¶à¿ÉÒÔ½ÓÊÜµÄÎÄ¼şÃèÊö·û

#define FD_NUM 3



extern map<int, int> ser_to_cli;
typedef enum _MsgType
>>>>>>> Stashed changes
=======
typedef enum _MsgType
>>>>>>> dev
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

class CMysql;

void Listenfd(evutil_socket_t fd, short int, void *arg);

void pthread_pool();

bool judge(int fd);

void server_start();

bool connect_server(int port, char *ip, int index);

void* thread_func(void *);

int select_server(int fd);

int deleteEvent(int epfd,int fd, struct epoll_event *event);

int addEvent(int epfd, int fd);

void ProcListenfd(evutil_socket_t fd, short , void *arg);

void* ReadThread(void *arg);

bool registe(int fd);

void* ReadThread(void *arg);

bool doLogin(int fd, char *name);

bool offline(int fd);//ä¸»åŠ¨æ‰“æ‹›å‘¼æ–­å¼€è¿˜æ˜¯ç›´æ¥æ–­å¼€--ã€‹æœåŠ¡å™¨çš„èµ„æº


<<<<<<< Updated upstream
//test the git
=======
int search_cli_to_ser_fd(int fd);

bool sendMesgFromDb(CMysql &db, int tofd, const char*name, int fd);

bool delSendedMsg(CMysql &db, const char *name);

bool delStateByfd(CMysql &db, int fd);

bool insertIntoMessage(const char *fromuser, const char *touser, const char* msg, CMysql &db);

char* findMesgByName(CMysql &db, const char*name);


<<<<<<< HEAD
>>>>>>> Stashed changes
=======
bool offline(int fd);//Ö÷¶¯´òÕĞºô¶Ï¿ª»¹ÊÇÖ±½Ó¶Ï¿ª--¡··şÎñÆ÷µÄ×ÊÔ´

void insert_clifd_serfd(int clientfd, int serverfd);

int search_cli_to_ser_fd(int fd);
>>>>>>> dev
