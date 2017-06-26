#include "head.h"

typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

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

//从lb接受消息，由于一台服务器只连接一个负载均衡器，所以对数据的收和发都是对同一个fd进行的
//1.当客户端请求的是注册时，mysql中增加user,回复的信息中将发来的fd添加
//2.当客户端是登陆的时候，向state表中插入数据，回复信息中增加其发来的fd
//3.当客户端是聊天的时候，查询对方是否在线，如果在线就将此包中的fd更换，将消息进行转发，
//  --如果不在线的话就将消息存储message<message，from, to>，向发送来的fd回复提醒消息

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
        
	size = recv(clientfd, recvbuf, 1024, 0);//server与fd的断开
	if (size <= 0)
        {
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
					//alter_state(int fd);//在states中按照fd将此项删除
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

