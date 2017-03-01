#include <iostream>
#include <string>
#include <map>
#include <list>
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
//lb的主线程
#define PORT 10000
#define IP  "172.0.0.1"
#define THREAD_NUM  10 
#define MAX 1000 //单线程中一个epoll最多可以接受的文件描述符
#define FD_NUM 3 //服务器的个数
int serverfd[FD_NUM];


static int id = 0;
map<int, int> fd;//用于记录fd和id的对应关系

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	sockaddr_in saddr;
	memset(saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = inet_addr(IP);

	int res = bind(sockfd, (sockaddr*)&saddr, sizeof(saddr));
	assert(res != -1);

	listen(sockfd, 5);

	list<int> worklist;
	pthread_mutex_t mutex;
	
	assert(res != -1);
	
	res = pthread_mutex_init(&mutex, NULL);
	
 	struct event_base *base = event_init();

	struct event *listen_event = event_new(base, listenfd, EV_READ|EV_PERSIST, Listenfd, NULL);
	
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    event_base_dispatch(base);
    event_free(listen_event);
    event_base_free(base);
    
    return 0;
}

int get_first(list<int> x)
{	
	if (x.empty())
	{
		return -1;
	}
	pthread_mutex_lock(&mutex);
	list<int>::iterator it = x.begin();
	int a = *it;
	x.pop_front();
	pthread_mutex_unlock(&mutex);
	return a;
}

void Listenfd(evutil_socket_t fd, short , void *arg)
{
	sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    //在此将fd传递给线程池中的线程  请求队列--先进先出
    pthread_mutex_lock(&mutex);
    worklist.push_back(clientfd);
    pthread_mutex_unlock(&mutex);
}

void pthread_pool()
{
	pthread_t tid;
	for(int i=0; i<THREAD_NUM; i++)
	{
		int res = pthread_create(&tid, NULL, thread_func, NULL);
		assert(res != -1);
	}
}

bool judge(int fd)//来自客户端就返回假
{
	for(int i=0; i<FD_NUM; ++i)
	{
		if(serverfd[i] == fd)
		{
			return true;
		}
	}
	return false;
}

void server_start()//向第index的服务器发送数据
{
	int port = 10000;//与聊天服务器相对应的客户端
	char *ip = "127.0.0.1";
	for(int i=0; i<FD_NUM; i++)
	{
		int ret = connect_server(port, ip, i);
		if (ret == -1)
		{
			cout<<"server "<<ret<<" can't open"<<endl;
			return ;
		}
	}
}

bool connect_server(int port, char *ip, int index)
{
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd == -1) {return false;}

	sockaddr_in caddr;
	memset(caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(clientfd, (sockaddr*)&caddr, sizeof(caddr));
	if (ret == -1) {return false;}
	
	serverfd[index] = clientfd; //连接上服务端
	return true;
}

void thread_func()
{
	int pfd = get_first();
	int epollfd =  epoll_create(6500);
	epoll_event events;
	int res = 0;
	while(true)
	{
		res = epoll_wait(epollfd, events, MAX, -1);
		char buff[1024];
		int ret = 0;
		for(int i=0; i<res; ++i)
		{
			Json::Value response;
			Json::Reader reader;
			Json::Value tempstr;
			Json::Value root;
			
			int fd = eventns[i].data.fd;
			
			ret = recv(fd, buff, 1024, 0);//后面有对其进行判断
			
			reader.parse(buff, tempstr);
			char * temp =tempstr["ID"];
			
			if(judge(fd))//服务器来的数据
			{
				int fd = get_fd(temp);//将字符串转化成正数，然后对正数进行map查找
				
				if (ret <= 0)
				{//与服务器断开了
					cout<<"server down"<<endl;
					return ;//设置 set_fd(-1) 
				}
				ret = send(fd, tempstr["mesg"].asString().c_str(), strlen(tempstr["mesg"].asString().c_str()), 0);
				if (ret <= 0){cout<<"send error"<<endl;}
			}
			else//客户端
			{
				int index = select_server();
				if (ret <= 0)
				{
					deleteEvent(epollfd, fd);//接收失败，断掉了连接
					continue;
				}
              ///////////////////////////////
				//对数据封装，发送给服务器
				int id_num = get_id();
				char temp[10];
				sprintf(temp, "%d", id_num);
							
				response["logo"] = temp;
				response["mesg"] = buff;
				//////////////////
				ret = send(serverfd[index], response.asString().c_str(), strlen(response.asString().c_str()), 0);     
				
				if (ret <= 0)
				{
					cout<<"error"<<endl;
				}   
		                
			}
			
		}
	}	
}

//如果是sockfd发来的信息呢--》记录fd, 怎么记录这个fd，采用什么格式来记录
//如何处理服务器连接lb时候的fd-->这个是我主动connect服务器吧

static int tag = 0;
int select_server(int fd)
{
	return tag = (tag+1)%3;
}

int deleteEvent(int epfd,int fd, struct epoll_event *event)
{
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
}

int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;//添加读事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
	setnonblocking(fd);//将文件描述符设置为非阻塞
}

int setnomblocking(int fd)//将文件描述符设置为非阻塞的
{
	int ols_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

//如今的问题是，如何使得lb的服务端能正确的向相应的客户端发送消息