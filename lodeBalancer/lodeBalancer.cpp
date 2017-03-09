#include "head.h"
#include<semaphore.h>
#include <unistd.h>

//lb的主线程
#define THREAD_NUM  1
#define MAX 50 //单线程中一个epoll最多可以接受的文件描述符
#define FD_NUM 3 //服务器的个数

int serverfd[FD_NUM];
static int id = 0;//map<int, int> fdLog;//用于记录fd和id的对应关系

list<int> worklist;
pthread_mutex_t mutex;

sem_t sem;

int ppfd[2];

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

	sem_init(&sem,0,0);
	//主线程之中建立无名管道，线程之间的文件描述符是共享的，让主线程收到c之后，而子线程中监视这个文件描述符号
	//一旦子线程中发现该描述符，就添加C到该线程的epool中
	 
	int res = pthread_mutex_init(&mutex, NULL);
	assert(res != -1);

 	struct event_base *base = event_init();

	struct event *listen_event = event_new(base, sockfd, EV_READ|EV_PERSIST, Listenfd, NULL);
	
    	event_add( listen_event, NULL );

    	int temp = pipe(ppfd);
    	assert(temp != -1);
    	
	server_start();//启动服务器
    	pthread_pool();//建立线程池
    
    	cout<<"lb started..."<<endl;
    
   	event_base_dispatch(base);
    	event_free(listen_event);
    	event_base_free(base);
    
    	return 0;
}
//是否存在主线程死亡，从线程活着

void Listenfd(evutil_socket_t fd, short int , void *arg)
{
    	CMysql db;
    	sockaddr_in client;
    	socklen_t len = sizeof(client);
    	int clientfd = accept(fd, (sockaddr*)&client, &len);
    	assert(clientfd != -1);
    	cout<<"new client connect server! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
    //在此将fd传递给线程池中的线程  请求队列--先进先出

	pthread_mutex_lock(&mutex);      //栈--替换队列    
    	worklist.push_back(clientfd);//向对列尾部添加数据
	db.insertInto_serverfd(id++, clientfd);//向map中添加fd--id的记录
    	pthread_mutex_unlock(&mutex);

    	char pipe_buff[10]={0};
    	sprintf(pipe_buff,"hh%d", clientfd);
	
   	if (write(ppfd[1], pipe_buff, strlen(pipe_buff)+1) == -1)
	{
		cout<<"write to pipe error"<<endl;
	}
	cout<<"pipe_buff is(in listenfd) "<<pipe_buff<<endl;

//	sem_post(&sem);//调试间使用
    	cout<<"new elem insert into worklist"<<endl;
}

void* thread_func(void *)
{
	CMysql db;//C++中全局的对象是否会能调用自己的成员函数
	int epollfd =  epoll_create(MAX);
	struct epoll_event events[MAX];       //sem_wait(&sem);//只有队列之中有新的数据的时候才会发生  
                                              //有新的数据证明有新的文件描述符  //若只是原来的文件描述符上有数据怎么办
	addEvent(epollfd, ppfd[0]);//主线程中创建的文件描述符，子线程中是否可见
	char buff[1024];
	int ret = 0;
	int res = 0;
  	Json::Value response;
	Json::Reader reader;
	Json::Value tempstr;
	Json::Value root;
//	if (sem_wait(&sem) == -1) //调试过程中使用，之后删除
//	{
//		if (errno != EINTR)
//			return NULL;
//	}
	while (true)
	{
		if((res = epoll_wait(epollfd, events, MAX, -1)) == -1)//阻塞在此出
		{        
			if(errno != EINTR)
				return NULL;
		}

		int pfd = 0;	
			//进不到循环中去	
		for(int i=0; i<res; ++i)
		{			
			int fd = events[i].data.fd;
			cout<<"in events fd is: "<<events[i].data.fd<<endl;
			if (events[i].data.fd == ppfd[0])
			{
			
				int pipebuff[10]={0};
				if((ret = read(ppfd[0], pipebuff, 10)) == -1)
				{
					cout<<" ai ya get pipe file error"<<endl;
				}
				//cout<<"worklist  "<<worklist<<endl;
			    	int t = get_first(worklist);
				
				addEvent(epollfd, t);
				cout<<"t is"<<endl;
				cout<<"buff is"<<pipebuff<<endl;

			}
			else if (judge(fd))//服务器来的数据：服务器来的数据上要先解封在发给客户端
			{						//考虑数据包的大小，防止分包
			    	ret = recv(fd, buff, 1024, 0);
				if (ret <= 0) {cout<<"error in recv from server"<<endl;}
		    		cout<<"recv buff"<<buff<<endl;
				if (ret <= 0)
				{
					cout<<"server down"<<endl;
					continue ;//设置 set_fd(-1) 
				}
				
				reader.parse(buff, tempstr);
				const char *temp =tempstr["ID"].asString().c_str();
				int fd = db.get_fd(atoi(temp));
				                   //将信息发送给相应的客户端
				cout<<"from server  to client"<<tempstr["mesg"].asString().c_str()<<endl;
				ret = send(fd, tempstr["mesg"].asString().c_str(), strlen(tempstr["mesg"].asString().c_str()), 0);
				if (ret <= 0){cout<<"send error"<<endl;}
			}
			else//客户端：数据加封，然后再进行发送
			{
				if(ret = recv(fd, buff, 1024, 0) <= 0)
					{cout<<"recv error from client"<<endl;}
				cout<<"recv buff"<<buff<<endl;
				int index = select_server(fd);
				if (ret <= 0)
				{
					deleteEvent(epollfd, fd, events);//接收失败，断掉了连接
					continue;
				}
      
				int id_num = db.get_id(fd);
				char temp[10];
				sprintf(temp, "%d", id_num);
							
				response["ID"] = temp;
				response["mesg"] = buff;
				
				cout<<"from client to server["<<index<<"]   "<<response.asString().c_str()<<endl;
				ret = send(serverfd[index], response.asString().c_str(), strlen(response.asString().c_str()), 0);     
				
				if (ret <= 0)
				{
					cout<<"error"<<endl;
				}                 
			}	
		}
	}	
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
	cout<<"worklist fd is: "<<a<<endl;
	x.pop_front();
	pthread_mutex_unlock(&mutex);
	return a;
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

//<key , value>--一个根据key找value,一个根据value找key.

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

int setnomblocking(int fd);//将文件描述符设置为非阻塞的
int addEvent(int epfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;//添加读事件
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		perror("epoll_ctl :listen_sock");
		exit(-1);
	}
	//setnomblocking(fd);//将文件描述符设置为非阻塞
}

int setnomblocking(int fd)//将文件描述符设置为非阻塞的
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

//如今的问题是，如何使得lb的服务端能正确的向相应的客户端发送消息
