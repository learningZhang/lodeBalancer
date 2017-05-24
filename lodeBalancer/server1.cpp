#include "head.h"
#include "memcached.h"
//服务器现在采取的是单线程--线程池工作模式


//数据库中CMysql对象的定义如何使得公用一个

int main()
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        cout<<"listenfd create fail!"<<endl;
        return -1;
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(-1 == bind(listenfd, (sockaddr*)&server, sizeof(server)))
    {
        cout<<"listenfd bind fail!"<<endl;
        return -1;
    }
    
    if(-1 == listen(listenfd, 20))
    {
        cout<<"listenfd listen fail!"<<endl;
        return -1;
    }

    pthread_t tid;
    struct event_base* base = event_init();   
    struct event *listen_event = event_new(base, listenfd,  EV_READ|EV_PERSIST, ProcListenfd, &tid);
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    event_base_dispatch(base);
    event_free(listen_event);
    event_base_free(base);
    
    return 0;
}

void ProcListenfd(evutil_socket_t fd, short , void *arg)
{
	//每当接收到信号的时候才会调用该函数，所以不能成为create_join的
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"
        <<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
 
    pthread_t tid;;
    int tag = pthread_create(&tid, NULL, ReadThread, (void*)clientfd);
	if (tag != 0)//则调用失败
	{
		cout<<"create pthread erron:"<<tag<<endl;
	}
//采用的是多线程，一个客户端来建立连接则开辟一个线程，此时的客户端只有要给lb
}

void* ReadThread(void *arg)
{
    CMysql db;
    
   	//CMemcached mem("127.0.0.1", 11211); //建立了一个mem的对象
   	
    int clientfd = (int)arg;
    while(true)
    {
        int size = 0;
        char recvbuf[1024]={0};
                
        Json::Reader reader;
        Json::Value root;
        Json::Value response;
<<<<<<< Updated upstream:lodeBalancer/chatserver.cpp
        
		size = recv(clientfd, recvbuf, 1024, 0);//server与lb的断开
		if (size <= 0)
    	{
	    	//将此server移除出去，然后从将states中的fd进行重新的分配
	    	//一个进程中，多个线程，之间的文件描述符是共享的，所以可以相互交换
	    	//// int re_arrage();//
        	cout<<"client connect fail!"<<errno<<endl;
        	close(clientfd);
        	return NULL;
    	}

  		cout<<"recvbuf is  "<<recvbuf<<endl;
		if (reader.parse(recvbuf, root))
		{
	    	int msgtype = root["msgtype"].asInt();
			
=======

       //服务器，等待客户端的连接，连接断开就将该线程关闭，清理数据库中该线程的信息
		size = recv(clientfd, recvbuf, 1024, 0);//server与lb的断开
		if (size == 0)
    	{
	    	//将此server移除出去，然后从将states中的fd进行重新的分配
	    	//一个进程中，多个线程，之间的文件描述符是共享的，所以可以相互交换
        	cout<<"client connect fail! in 81 !!"<<errno<<endl;
        	close(clientfd);
        	pthread_exit((void*)1);
        	//服务器与lb之间的连接，一共有三个服务器与lb连接，而一个lb和服务器是属于一个连接
        	//只是占用了一个文件描述符

        	//客户端来连接服务器，这里服务器的直接连接的客户端只有服务器一个
    	}
    	else if(size<0) 
    	{
	    	cout<<"接受消息失败"<<endl;
	    	cout<<"errno is: "<<errno<<endl;
	    	continue;
    	}

  		cout<<"recvbuf is  "<<recvbuf<<endl;
		if (reader.parse(recvbuf, root))
		{
	    	int msgtype = root["msgtype"].asInt();
>>>>>>> Stashed changes:lodeBalancer/server1.cpp
            switch(msgtype)
            {	            
                case EN_MSG_LOGIN://登陆
                {
                	response["msgtype"] = EN_MSG_ACK;
                	response["FD"] = root["FD"].asInt();
                	
             	    string name = root["name"].asString();
                    if(db.queryPasswd(name.c_str(), root["pwd"].asString().c_str()))
                    {
                        response["ackcode"] = "ok";
        				if (!db.insertIntoStates(name.c_str(), root["FD"].asInt()))
                        {
	                        cout<<"insertIntoStates error"<<endl;
                        }
                    }
                    else
                    {
                        response["ackcode"] = "error";
              	    } 
<<<<<<< Updated upstream:lodeBalancer/chatserver.cpp
                    send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
<<<<<<< HEAD:lodeBalancer/chatserver.cpp
=======
                    size = send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
					if (size == -1)
					{
						cout<<"send error"<<endl;
					}
					
					int tofd = root["FD"].asInt();
					if(sendMesgFromDb(db, tofd, root["name"].asString().c_str(), clientfd))//发送留言信息
					{
						if(!delSendedMsg(db ,name.c_str()))
							cout<<"del sended message error"<<endl;
					}
>>>>>>> Stashed changes:lodeBalancer/server1.cpp
=======
					sendMesgFromDb(root["name"].asString.c_str(), clientfd);//发送留言信息
>>>>>>> dev:lodeBalancer/server1.cpp
                }
                break;
                
                case EN_MSG_CHAT://chat with other
                {	
					int tempfd = db.getStates(root["to"].asString().c_str());//error
					cout<<"to "<<root["to"].asString().c_str()<<"  tempfd is  "<<tempfd<<endl;
 
					if(tempfd != -1)//对方在线
					{
						response["msgtype"] = EN_MSG_CHAT;
						response["from"] = root["from"].asString();
						response["msg"] = root["msg"].asString();
						response["FD"] = tempfd;
					}
					else
					{
<<<<<<< HEAD:lodeBalancer/chatserver.cpp
<<<<<<< Updated upstream:lodeBalancer/chatserver.cpp
=======
						insertIntoMessage(root["from"].asString().c_str(), 
								root["to"].asString().c_str(),root["msg"].asString().c_str(), db);//留言信息存储	
							//磁盘上操作太慢，将其加入到epoll,如何添加
>>>>>>> Stashed changes:lodeBalancer/server1.cpp
=======
						insertIntoMessage(root["from"].asString().c_str(), 
								root["to"].asString().c_str(),root["msg"].asString().c_str());//留言信息存储	
							//磁盘上操作太慢，将其加入到epoll,如何添加
>>>>>>> dev:lodeBalancer/server1.cpp
						response["FD"] = root["FD"].asInt();
						response["msgtype"] = EN_MSG_ACK;
						response["ackcode"] = "sorry, he is not zaixian";	
					}
					size = send(clientfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0);
					if (size < 0)
					{
						cout<<"send error"<<endl;
						cout<<"erron is"<<errno;
					}
                }
                break;

              	case EN_MSG_REGISTER://注册
				{
					string name = root["name"].asString();
					string passwd = root["passwd"].asString();
					string email = root["email"].asString();
					
		   		  	response["msgtype"] = EN_MSG_ACK;
		       		response["FD"] = root["FD"].asInt();
		       		
		   		   	if (!db.insertIntoUser(name.c_str(), passwd.c_str(), email.c_str()))
					{
						cout<<"do it fail"<<endl;
						response["ackcode"] = "no";
					}
					else
					{
						response["ackcode"] = "yes";
					}
		        	size = send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
					if (size == -1)
					{
						cout<<"send error"<<endl;
					}
				}     
      			break;
      			
	       		case EN_MSG_OFFLINE://离线
<<<<<<< Updated upstream:lodeBalancer/chatserver.cpp
				{
<<<<<<< HEAD:lodeBalancer/chatserver.cpp
					close(clientfd);
					//alter_state(int fd);//在states中按照fd将此项删除
=======
				{		//<serverfd ,lbfd>
					//close(clientfd);
					//现在的服务器只是连接了一个客户端，即lb
					delStateByfd(db, clientfd);//user state message
>>>>>>> Stashed changes:lodeBalancer/server1.cpp
=======
					close(clientfd);		
					delStateByfd(db, clientfd);//bool alter_state(int fd);//在states中按照fd将此项删除
>>>>>>> dev:lodeBalancer/server1.cpp
					cout<<"server offlien"<<endl;
					pthread_exit((void*)1);//服务器是采用的多线程	
				}
				break;
            }
        }
    }
}

<<<<<<< Updated upstream:lodeBalancer/chatserver.cpp
void ProcListenfd(evutil_socket_t fd, short , void *arg)
{
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"
        <<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;
 
    pthread_t tid;
    pthread_create(&tid, NULL, ReadThread, (void*)clientfd);
}

<<<<<<< HEAD:lodeBalancer/chatserver.cpp
=======
#define 1024 MESSAGE_MAX_LENGTH
bool sendMesgFromDb(CMysql &db, int tofd, const char*name, int fd)//寻找，怎么进行触发，登陆时候
{
	Json::Value response;
	char *buff;
	char *temp=(char *str)malloc(MESSAGE_MAX_LENGTH*sizeof(char));
	if (temp == NULL) return false;
	
	if (!findMesgByName(db, name, temp, MESSAGE_MAX_LENGTH))//得到字符串，在db中对字符串进行包装，再解封
	{
		free(temp);
		return false;
	}
	else
	{
		const char *temp = "-";
		char *fromuser = strtok(buff, temp);
		char *msg = strtok(NULL, temp);
	
		response["FD"] = tofd;//找到在负载均衡器中的文件描述符
		response["msgtype"] = EN_MSG_ACK;
		response["msg"] = buff;
		response["from"] = fromuser;

		int x = send(fd, response.toStyledString().c_str(), 
			strlen(response.toStyledString().c_str()), 0);
		free(temp);
		if (x == -1)
		{
			return false;
		}
		return true;//发送成功就将其从数据库中删除
	}
}
>>>>>>> Stashed changes:lodeBalancer/server1.cpp
=======
//bool delInMessage(cosnt char *name);
//char * findMesgByName(const char*name);

void sendMesgFromDb(const char*name, int fd)//寻找，怎么进行触发，登陆时候
{
	Json::Value response;
	const char *buff;
	if ((buff == findMesgByName(name)) == NULL)//得到字符串，在db中对字符串进行包装，再解封
	{
		return ;
	}
	else
	{
		const char *temp = ",";

		char *fd = strtok(buff, temp);
		char *msg = strtok(buff, temp);
		char *from =strtok(buff, temp);
	
		response["FD"] = fd;
		response["msgtype"] = EN_MSG_ACK;
		response["msg"] = buff;
		response["from"] = from;

		int x = send(fd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str()), 0);
		if (x<0)
		{
			return ;
		}
		else
		{
			delInMessage(name);//发送成功就将其从数据库中删除
		}
	}
}
>>>>>>> dev:lodeBalancer/server1.cpp
