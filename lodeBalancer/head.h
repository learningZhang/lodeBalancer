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

<<<<<<< Updated upstream
class CMysql
=======

#define THREAD_NUM 3
#define MAX 50 //单线程中一个epoll最多可以接受的文件描述符

#define FD_NUM 3



extern map<int, int> ser_to_cli;
typedef enum _MsgType
>>>>>>> Stashed changes
{
	public:
		CMysql();
		bool insertIntoUser(const char *name, const char *passwd, const char *call);
		bool queryPasswd(const char *name, const char *passwd);
		bool deleteUser(const char *name);
		int getStates(const char *name);
		bool insertIntoStates(const char *name, int fd);
		bool insertInto_serverfd(int fd, int id);
		int  get_fd(int id);
		int  get_id(int fd);
		int deleteFromStates(char *name);
		//
	private:
		MYSQL *pcon;
	 	MYSQL_RES *pres;
		MYSQL_ROW row;
		
	 	const char *ip;
		const char *user;
		const char *passwd;
		unsigned short port;
};

int get_first(list<int> &x);

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

bool offline(int fd);//涓诲姩鎵撴嫑鍛兼柇寮�杩樻槸鐩存帴鏂紑--銆嬫湇鍔″櫒鐨勮祫婧�


<<<<<<< Updated upstream
//test the git
=======
int search_cli_to_ser_fd(int fd);

bool sendMesgFromDb(CMysql &db, int tofd, const char*name, int fd);

bool delSendedMsg(CMysql &db, const char *name);

bool delStateByfd(CMysql &db, int fd);

bool insertIntoMessage(const char *fromuser, const char *touser, const char* msg, CMysql &db);

char* findMesgByName(CMysql &db, const char*name);


>>>>>>> Stashed changes
