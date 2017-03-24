#include"head.h"

#define THREAD_NUM 3
#define MAX 50 //单线程中一个epoll最多可以接受的文件描述符

#define FD_NUM 3
int serverfd[FD_NUM];//服务器的fd
int ppfd[2];

extern map<int, int> ser_to_cli;

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
	//create_pthread_for_message();

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
