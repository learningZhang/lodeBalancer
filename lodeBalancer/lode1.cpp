<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
<<<<<<< Updated upstream:lodeBalancer/lodeBalancer.cpp
#include"head.h"

#define THREAD_NUM 3
#define MAX 50 //单线程中一个epoll最多可以接受的文件描述符

#define FD_NUM 3
int serverfd[FD_NUM];
int ppfd[2];

map<int, int> ser_to_cli;
void insert_clifd_serfd(int clientfd, int serverfd);
int search_cli_to_ser_fd(int fd);

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(10001);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (-1 == bind(sockfd, (sockaddr*)&saddr, sizeof(saddr)))
	{
		cout<<"error in bind"<<endl;
		return -1;
	}
	
	if (-1 == listen(sockfd, 5))
	{
		cout<<"error in listen"<<endl;
		return -1;
	}
	
	server_start();
    	assert(pipe(ppfd) != -1);
	pthread_pool();

	
 	struct event_base *base = event_init();
	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);
    	event_add( listen_event, NULL );

    	cout<<"lodebalancer is started..."<<endl;
    
   	event_base_dispatch(base);
    	event_free(listen_event);
    	event_base_free(base);
    	return 0;
}

void* thread_func(void *)
{
	CMysql db;
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       
	
	addEvent(epollfd, ppfd[0]);

	for(int i=0; i<FD_NUM; ++i)
	{
		addEvent(epollfd, serverfd[i]);
	}

	char buff[1024];
	int ret = 0;
	int res = 0;

  	Json::Value response;
	Json::Reader reader;
	Json::Value root;
	
	while (true)
	{
		if ((res = epoll_wait(epollfd, events, MAX, -1)) == -1)
		{        
			if (errno != EINTR)
			{
				return NULL;
			}
		}

		int pfd = 0;		
		for (int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			if (events[i].data.fd == ppfd[0])
			{
				char pipebuff[10];
				if((ret = read(ppfd[0], pipebuff, 10)) == -1)
				{
					cout<<"get pipe file error"<<endl;
					continue;
				}
				addEvent(epollfd, atoi(pipebuff));
				cout<<"pipe buff is "<<pipebuff<<endl;
			}
			else if (judge(fd))
			{				
				ret = recv(fd, buff, 1024, 0);
				cout<<"recv from server "<<buff<<endl;
				if (ret <= 0)//与服务器断开了联系,如何善后,有存储的<客户端，服务器>,将其进行重新分配
				{
					
					cout<<"server down"<<endl;
					deleteEvent(epollfd, fd, events);//int new_given();
					//continue;
					break;
				}

				if (reader.parse(buff, root))
				{
					fd = root["FD"].asInt();
					ret = send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);     
					if (ret <= 0)
					{
						cout<<"error"<<endl;
					}   
				}
			}
			else//信息来自客户端
			{		  
				int serfd = 0;  	
				ret = recv(fd, buff, 1024, 0);
				if (ret <= 0)
				{
					cout<<"server down"<<endl;
					serfd = search_cli_to_ser_fd(fd);
					
					if (serfd  == -1)//对方还没有获得处理其的服务器
					{
						cout<<"a client over and dont have serverfd"<<endl;
					}
					else
					{
						response["FD"] = fd;
						response["msgtype"] = offline;
					
						send(serfd, response.toStyledString().c_str(), 
						  strlen(response.toStyledString().c_str())+1, 0);//告诉server去修改state表中的用户状态
					}
					
					deleteEvent(epollfd, fd, events);//接收失败，断掉了连接
					continue;
				}

				cout<<"recv from client"<<buff<<endl;

				if(reader.parse(buff, response))
				{
					serfd = search_cli_to_ser_fd(fd);
					if (serfd == -1)
					{
						int index = select_server(fd);//选择服务器
						insert_clifd_serfd(fd, serverfd[index]);
						serfd = serverfd[index];
					}
					response["FD"] = fd;
					ret = send(serfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1, 0);     
					if (ret <= 0)
					{
						cout<<"error"<<endl;
					}   
				}              
			}	
		}
	}	
}


void Listenfd(evutil_socket_t fd, short int , void *arg)//主线程
{
	CMysql db;
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    
    assert(clientfd != -1);
    cout<<"new client connected! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    char pipe_buff[10]={0};
    sprintf(pipe_buff,"%d", clientfd);
	
   	if (write(ppfd[1], pipe_buff, strlen(pipe_buff)+1) == -1)
	{
		cout<<"write to pipe error"<<endl;
	}
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
		cout<<"server["<<i<<"]   open"<<endl;
	}
}

bool connect_server(int port, char *ip, int index)
{
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientfd == -1) {return false;}

	sockaddr_in caddr;
	memset(&caddr, 0, sizeof(caddr));
	
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(clientfd, (sockaddr*)&caddr, sizeof(caddr));
	if (ret == -1) {return false;}
	
	serverfd[index] = clientfd; //连接上服务端
	return true;
}

void pthread_pool()
{
	pthread_t tid;
	for(int i=0; i<THREAD_NUM; i++)
	{
		int res = pthread_create(&tid, NULL, thread_func, NULL);
		assert(res != -1);
		cout<<"pthread["<<i<<"]  open"<<endl;
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

static int tag = 0;
int select_server(int fd)
{
	return tag = (tag+1)%3;
}

int deleteEvent(int epfd,int fd, struct epoll_event *event)
{
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
}

int setnomblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		perror("epoll_ctl :listen_sock");
		exit(-1);
	}
}

int search_cli_to_ser_fd(int fd)
{
	map<int, int>::iterator it = ser_to_cli.find(fd);
	if (it == ser_to_cli.end())
	{
		return -1;
	}
	return it->second;
}
//根据客户端的fd的值来搜索当前数据库中是否有与之对应的服务器端，若是有的话就查找服务器的fd，否则就返回-1,让客户端进行分配

void insert_clifd_serfd(int clientfd, int serverfd)//将客户端来的作为主键，插入<clientfd,serverfd>
{
	ser_to_cli[clientfd] = serverfd;
}

=======
#include"head.h"
int serverfd[FD_NUM]={0};//服务器的fd
int ppfd[2]={0};
=======
#include"head.h"

#define THREAD_NUM 3
#define MAX 50 //单线程中一个epoll最多可以接受的文件描述符

#define FD_NUM 3
int serverfd[FD_NUM];//服务器的fd
int ppfd[2];

extern map<int, int> ser_to_cli;
>>>>>>> dev:lodeBalancer/lode1.cpp

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(10001);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (-1 == bind(sockfd, (sockaddr*)&saddr, sizeof(saddr)))
	{
		cout<<"error in bind"<<endl;
		return -1;
	}
	
	if (-1 == listen(sockfd, 5))
	{
		cout<<"error in listen"<<endl;
		return -1;
	}
	
	server_start();
    assert(pipe(ppfd) != -1);
	pthread_pool();
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
=======
	//create_pthread_for_message();
>>>>>>> dev:lodeBalancer/lode1.cpp

 	struct event_base *base = event_init();
	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);    	event_add( listen_event, NULL );

   	cout<<"lodebalancer is started..."<<endl;
    
   	event_base_dispatch(base);
   	event_free(listen_event);
   	event_base_free(base);
   	return 0;
}

<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
void* thread_func(void *)
{	
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       //创建一个数组，用于接受传递回来的事件值
=======

void* thread_func(void *)
{
	CMysql db;
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       
>>>>>>> dev:lodeBalancer/lode1.cpp
	
	addEvent(epollfd, ppfd[0]);

	for(int i=0; i<FD_NUM; ++i)
	{
		addEvent(epollfd, serverfd[i]);
	}

	char buff[1024];
	int ret = 0;
	int res = 0;

  	Json::Value response;
	Json::Reader reader;
	Json::Value root;
	
	while (true)
	{
		if ((res = epoll_wait(epollfd, events, MAX, -1)) == -1)
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
		{        ///传递进去events数组,MAX为数组的长度，然后进行监听，MAX表示一次监听最大的文件描述符的个数
=======
		{        
>>>>>>> dev:lodeBalancer/lode1.cpp
			if (errno != EINTR)
			{
				return NULL;
			}
		}
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
		//两种情况，一个是客户端和lb断开联系，一个是服务器与lb断开联系，
		//客户端与lb断开来连接，将客户端的信息清除，从epoll中删除该客户端的fd,让客户端重新连接
		//lb与服务器断开连接，lb中是epoll结构，而服务器中是多线程，是一个断开连接
		//多个服务器进程在运行，探测到某个服务器突然断开就要重新分配该服务器中所有
		//连接的fd给其他服务器

		//进程与进程之间断开了连接
		//两个机器之间断开了连接

		//如何知道是服务器断开连接还是只是进程之间断开连接----心跳包

		//现在所能做的就是假设只是个别的连接断开了

		int delEvent[100];
		int f = 0;
=======

>>>>>>> dev:lodeBalancer/lode1.cpp
		int pfd = 0;		
		for (int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			if (events[i].data.fd == ppfd[0])
			{
				char pipebuff[10];
				if((ret = read(ppfd[0], pipebuff, 10)) == -1)
				{
					cout<<"get pipe file error"<<endl;
					continue;
				}
				addEvent(epollfd, atoi(pipebuff));
				cout<<"pipe buff is "<<pipebuff<<endl;
			}
			else if (judge(fd))
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
			{		//服务器---lb
				ret = recv(fd, buff, 1024, 0);
				cout<<"recv from server "<<buff<<endl;
				//首先客户端和lb断开，客户端发送消息告诉服务器断开连接
				//然后服务器就对该采取行动
				if (ret == 0)
				{
					cout<<"with server break out"<<endl;
					//int new_given();
					continue;
					//lb是多线程+epoll模型，连接了多个服务器
					//一个线程一个epoll,有请求之后，查询它是属于哪一个服务器管的，然后找到该服务器的fd
	
					//重新分配，然后继续正常运行
					//应该将所连接到该服务器上的所有客户端重新分配给剩下的客户端，然后continue;
					
				}
				else if(ret<0)
				{
					cout<<"recv error"<<endl;
					continue;
=======
			{				
				ret = recv(fd, buff, 1024, 0);
				cout<<"recv from server "<<buff<<endl;
				if (ret <= 0)//与服务器断开了联系,如何善后,有存储的<客户端，服务器>,将其进行重新分配
				{					
					cout<<"server down"<<endl;
					deleteEvent(epollfd, fd, events);//int new_given();
					//continue;
					break;
>>>>>>> dev:lodeBalancer/lode1.cpp
				}

				if (reader.parse(buff, root))
				{
					fd = root["FD"].asInt();
					ret = send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);     
					if (ret <= 0)
					{
						cout<<"error"<<endl;
					}   
				}
			}
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
			else//客户端和lb断开了连接
			{		
				int serfd = 0;  	
				ret = recv(fd, buff, 1024, 0);
				if (ret == 0)
				{
					cout<<"with client break out"<<endl;
					serfd = search_cli_to_ser_fd(fd);//建立和一致性哈希就可以根据键来找到对应的服务器fd
=======
			else//信息来自客户端
			{		  
				int serfd = 0;  	
				ret = recv(fd, buff, 1024, 0);
				if (ret <= 0)
				{
					cout<<"server down"<<endl;
					serfd = search_cli_to_ser_fd(fd);
					
>>>>>>> dev:lodeBalancer/lode1.cpp
					if (serfd  == -1)//对方还没有获得处理其的服务器
					{
						cout<<"a client over and dont have serverfd"<<endl;
					}
					else
					{
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
						response["FD"] = fd;//向服务器报信
						response["msgtype"] = offline;
						send(serfd, response.toStyledString().c_str(), 
						  strlen(response.toStyledString().c_str())+1, 0);//告诉server去修改state表中的用户状态
					}
					delEvent[f++] = fd;
					continue;
				}
				else if (ret < 0)
				{
					cout<<"recv error"<<endl;
					cout<<"errno is :"<<errno<<endl;
=======
						response["FD"] = fd;
						response["msgtype"] = offline;
					
						send(serfd, response.toStyledString().c_str(), 
						  strlen(response.toStyledString().c_str())+1, 0);//告诉server去修改state表中的用户状态
					}
					
					deleteEvent(epollfd, fd, events);//接收失败，断掉了连接
>>>>>>> dev:lodeBalancer/lode1.cpp
					continue;
				}

				cout<<"recv from client"<<buff<<endl;
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
=======

>>>>>>> dev:lodeBalancer/lode1.cpp
				if(reader.parse(buff, response))
				{
					serfd = search_cli_to_ser_fd(fd);
					if (serfd == -1)
					{
						int index = select_server(fd);//选择服务器
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
						insert_clifd_serfd(fd, serverfd[index]);//一致性哈希替换
=======
						insert_clifd_serfd(fd, serverfd[index]);
>>>>>>> dev:lodeBalancer/lode1.cpp
						serfd = serverfd[index];
					}
					response["FD"] = fd;
					ret = send(serfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1, 0);     
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
					if (ret == -1)
=======
					if (ret <= 0)
>>>>>>> dev:lodeBalancer/lode1.cpp
					{
						cout<<"error"<<endl;
					}   
				}              
			}	
		}
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
		if (f != 0)
		{
			for (int i=0; i<f; ++i)
			{
				deleteEvent(epollfd, delEvent[i], events);
			}		
		}
=======
>>>>>>> dev:lodeBalancer/lode1.cpp
	}	
}


void Listenfd(evutil_socket_t fd, short int , void *arg)//主线程
{
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp
=======
	CMysql db;
>>>>>>> dev:lodeBalancer/lode1.cpp
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    
    assert(clientfd != -1);
    cout<<"new client connected! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    char pipe_buff[10]={0};
    sprintf(pipe_buff,"%d", clientfd);
	
   	if (write(ppfd[1], pipe_buff, strlen(pipe_buff)+1) == -1)
	{
		cout<<"write to pipe error"<<endl;
	}
}

//留言功能？？？？？？？？？？
//按照留言中的姓名进行查找，如果此人存在就对其发送信息
//数据库中查找，是要开辟新的线程给其还是在线程中利用文件描述符制作统一事件源

//如果在此方向发现了客户端掉线就发送消息给服务器端，服务器端用此来对数据库进行操作state

//lodebalance的线程函数执行的是什么--》从客户端来的信息
//1.获得到文件描述符，如果是监听套接字，就accept，如果是连接套接字
//--1.如果是登陆信息，加上自己的文件描述符，发送给服务器--》服务器进行存储
//--2.如果是注册信息，加上自己的文件描述符，回复注册成功
//--3.如果是聊天信息，加上自己的文件描述符<如果对方在线，返回来的时候替换成对方的fd，如果对方不在线就给此人发送提示信息>

//如何知道这个客户端去了哪一个服务器，一个客户端只能并且一直连接一台服务器
//添加表，用来记录服务器和客户端的对应关系
<<<<<<< HEAD:lodeBalancer/lodeBalancer.cpp


//一个服务器与lb之间的连接只有一个，当期之间发生断开连接的时候lb要将该服务器所连接
//至其上的所有客户端重新分配
>>>>>>> Stashed changes:lodeBalancer/lode1.cpp
=======
>>>>>>> dev:lodeBalancer/lode1.cpp
