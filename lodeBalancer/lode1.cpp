#include"head.h"
int serverfd[FD_NUM]={0};//服务器的fd
int ppfd[2]={0};

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
	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);    	event_add( listen_event, NULL );

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
			else//客户端和lb断开了连接
			{		
				int serfd = 0;  	
				ret = recv(fd, buff, 1024, 0);
				if (ret == 0)
				{
					cout<<"with client break out"<<endl;
					serfd = search_cli_to_ser_fd(fd);//建立和一致性哈希就可以根据键来找到对应的服务器fd
					if (serfd  == -1)//对方还没有获得处理其的服务器
					{
						cout<<"a client over and dont have serverfd"<<endl;
					}
					else
					{
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
					if (ret == -1)
					{
						cout<<"error"<<endl;
					}   
				}              
			}	
		}
		if (f != 0)
		{
			for (int i=0; i<f; ++i)
			{
				deleteEvent(epollfd, delEvent[i], events);
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


//一个服务器与lb之间的连接只有一个，当期之间发生断开连接的时候lb要将该服务器所连接
//至其上的所有客户端重新分配