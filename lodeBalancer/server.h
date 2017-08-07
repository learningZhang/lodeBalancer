#define THREAD_NUM 3
#define MAX 500 //单线程中一个epoll最多可以接受的文件描述符
#define FD_NUM 3

typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

#define  MESSAGE_MAX_LENGTH 1024

void  ProcListenfd(evutil_socket_t fd, short , void *arg);

void* ReadThread(void *arg);

bool  insertIntoMessage(const char *fromuser, const char *touser, const char* msg, CMysql &db);

bool  delStateByfd(CMysql &db, int fd);

bool  delAllStateInfo(CMysql &db);

bool  findMesgByName(CMysql &db, const char*name, char *str, int length);

int   searchFd(CMysql &db, const char *name);

bool  sendMesgFromDb(CMysql &db, int tofd, const char*name, int fd);
