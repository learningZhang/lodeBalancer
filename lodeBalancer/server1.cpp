#include "head.h"

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
    
    struct event_base* base = event_init();   
    struct event *listen_event = event_new(base, listenfd,  EV_READ|EV_PERSIST, ProcListenfd, NULL);
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    
    event_base_dispatch(base);
    event_free(listen_event);
    event_base_free(base);
    return 0;
}

void* ReadThread(void *arg)
{
    CMysql db;
    int clientfd = (int)arg;
    while(true)
    {
        int size = 0;
        char recvbuf[1024]={0};
                
        Json::Reader reader;
        Json::Value root;
        Json::Value response;
        
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
                    send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
					sendMesgFromDb(root["name"].asString.c_str(), clientfd);//发送留言信息
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
						insertIntoMessage(root["from"].asString().c_str(), 
								root["to"].asString().c_str(),root["msg"].asString().c_str());//留言信息存储	
							//磁盘上操作太慢，将其加入到epoll,如何添加
						response["FD"] = root["FD"].asInt();
						response["msgtype"] = EN_MSG_ACK;
						response["ackcode"] = "sorry, he is not zaixian";	
					}
					send(clientfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0);
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
		        	send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
				}     
      			break;
      			
	       		case EN_MSG_OFFLINE://离线
				{
					close(clientfd);		
					delStateByfd(db, clientfd);//bool alter_state(int fd);//在states中按照fd将此项删除
					cout<<"server offlien"<<endl;
				}
				break;
				
            }
        }
    }
}

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