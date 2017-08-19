#include"head.h"
int serverfd[FD_NUM]={0};//服务器的fd
int ppfd[2]={0};

//1.如果由主线程进行监听活动，得到accept后将事件通过管道发送给子线程，那么就不存在惊群事件
//2.如果由各个线程进行accept，且将监听socket放到epoll中，则就会发生惊群现象，则这时就是变化
//了模式，由高效的高并发模型转变成了nginx的模型，主线程负责其它事件，而从线程进行整个从监听
//到处理请求的所有过程

bool getAcceptMutex(int disable, pthread_mutex_t *mutex，bool lock，int fd, int epollfd)
{//如何进行动态调节disable的值，
	if (disable > 0)
	{
		--disable;
	}
	else
	{
		if (lock==fasle && pthread_mutex_trylock(mutex)==0)//不阻塞的获得锁
		{ //该锁没有被获得，且锁获得成功
			lock=true;
			addEvent(epollfd,fd);//将监听socket加入到epoll中
			return true;
		}
	}
	return false;		
}

bool offAcceptMutex()
{
	if (lock == true && pthread_mutex_unlock(mutex)==0)
	{
		return true;
	}
	else
	return fasle;
}

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
	
	pthread_mutex_t  mutex;
	int ret = pthread_mutex_init(mutex,NULL);//对锁进行初始化

	assert(ret<0);
 	//struct event_base *base = event_init();
	//struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);    	event_add( listen_event, NULL );

   	cout<<"lodebalancer is started..."<<endl;
    //则nginx中主进程负责接收信号，并且监视进程，平稳的更新各个进程
    //而在主线程中进行线程池的初始化，信号的接收，监听socket的创建
    
   	//event_base_dispatch(base);
	//ret = pthread_mutex_destroy(mutex);动态申请的需要
	//assert(ret<0);
   	//event_free(listen_event);
   	
   	//event_base_free(base);
   	return 0;
}
/*
void Listenfd(evutil_socket_t fd, short int , void *arg)//主线程
{
	CMysql db;
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    				//主线程负责进行监听，有事件发生就将该事件添加到
    assert(clientfd != -1);
    cout<<"new client connected! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    char pipe_buff[10]={0};
    sprintf(pipe_buff,"%d", clientfd);
	
   	if (write(ppfd[1], pipe_buff, strlen(pipe_buff)+1) == -1)
	{
		cout<<"write to pipe error"<<endl;
	}
}
*/
////////////
void* thread_func(void *)
{
	CMysql db;
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       
	int acceptClientNum=0;
	
	int disable = MAX/8-acceptClientNum;//平衡因子

	//addEvent(epollfd, ppfd[0]);
    //如果有事件发生主进程向该事件中发送
	
	//如果大于该值则表示该线程的负载已经很高，则会进行自减，
	//如果整体都为负值，通过调用会逐渐的变为正值，所以在一段时间内可能会不接受客户端的连接
	//如果小于0就会进行accept来接受客户端的连接
	
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
//在此处进行争抢，如果拿到了这个锁之后就会将自己的文件描述符号加入进去，否则就拿不到
//互斥锁封装的的条件变量和信号量，如果拿到锁就进行添加退出，否则立即返回不进行阻塞
//互斥锁有两种处理方式，一个阻塞lock一个非阻塞trylock
		getAcceptMutex();//尝试去获得这把锁		
		
		if ((res = epoll_wait(epollfd, events, MAX, -1)) == -1)
		{        
			if (errno != EINTR)
			{
				return NULL;
			}
		}

		int delEvent[100];
		int f = 0;
		int pfd = 0;		
		for (int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			if (events[i].data.fd == ppfd[0])
			{
				//如果是该线程获得了这把锁的话，就将这把锁进行释放
				//如果该线程没有获得锁，其会进来吗

				accept();//进行连接
				++acceptClientNum;
				offAcceptMutex();//进行锁的释放
				//如果在处理事件过程中有accept发生怎么办，此时的结构是nginx中的结构模型
				
				/*
				char pipebuff[10];
				if((ret = read(ppfd[0], pipebuff, 10)) == -1)
				{
					cout<<"get pipe file error"<<endl;
					continue;
				}
				addEvent(epollfd, atoi(pipebuff));
				cout<<"pipe buff is "<<pipebuff<<endl;
				*/
			}
			else if (judge(fd))
			{		
				ret = recv(fd, buff, 1024, 0);
				cout<<"recv from server "<<buff<<endl;
				
				if (ret == 0)
				{
					cout<<"with server break out"<<endl;
					//int new_given();
					continue;
					
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
					serfd = search_cli_to_ser_fd(fd);
					if (serfd  == -1)
					{
						cout<<"a client over and dont have serverfd"<<endl;
					}
					else
					{
						response["FD"] = fd;
						response["msgtype"] = offline;
						send(serfd, response.toStyledString().c_str(), 
						  strlen(response.toStyledString().c_str())+1, 0);
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
						int index = select_server(fd);
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

