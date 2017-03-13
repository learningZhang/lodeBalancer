#include "head.h"

typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

void* ReadThread(void *arg)
{
    CMysql db;
    int clientfd = (int)arg;
    while(true)
    {//接收client发送的消息，处理，响应
        int size = 0;
        char recvbuf[1024]={0};
        
        Json::Reader reader;
        Json::Value root;
        Json::Value response;

	size = recv(clientfd, recvbuf, 1024, 0);
	if (size <= 0)//when a client closed,the size is 0
        {
            cout<<"errno:"<<errno<<endl;
            cout<<"client connect fail!"<<endl;
            close(clientfd);
            return NULL;
        }
       cout<<"recvbuff is : "<<recvbuf<<endl;
  		if (reader.parse(recvbuf, root))
		{
			cout<<"name :"<<root["name"].asString().c_str()<<endl;

           	int msgtype = root["msgtype"].asInt();
			cout<<"msgtype  is: "<<msgtype<<endl;

            switch(msgtype)
            {
	        	response["ID"] = root["ID"].asString().c_str();//将标签重新加上	            
                case EN_MSG_LOGIN:
                {
                    response["msgtype"] = EN_MSG_ACK;
                    string name = root["name"].asString();
                    if(db.queryPasswd(name.c_str(), root["pwd"].asString().c_str()))
                    {
                        response["ackcode"] = "ok";
                        clientfd = db.getStates(name.c_str());
                        //chat[name.c_str()] = clientfd;//用户登陆成功之时记录键值对<name, fd>
                        cout<<"name"<<name<<"  fd"<<clientfd<<endl;
                    }
                    else
                    {
                        response["ackcode"] = "error";
              	    }
                    cout<<"response:"<<response.toStyledString()<<endl;
                    send(clientfd, response.toStyledString().c_str(),
                        strlen(response.toStyledString().c_str())+1, 0);
			cout<<"login send : "<<response.toStyledString().c_str()<<endl;
                }
                break;
                
                case EN_MSG_CHAT:
                {
                    cout<<root["from"]<<" => "<<root["to"]<<":"<<root["msg"]<<endl;    
                   //	map<string, int>::iterator it = chat.find(root["to"].asString());
					clientfd = db.getStates(root["to"].asString().c_str());
					if(clientfd != -1)
					{
						response["msgtype"] = EN_MSG_CHAT;
						response["from"] = root["from"].asString();
						response["msg"] = root["msg"].asString();
						send(clientfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0);
						cout<<"chat send : "<<response.toStyledString().c_str()<<endl;
					}
					else
					{
						cout<<"should do it"<<endl;
						//response["msgtype"] = ;
						//response["msg"] = "error";
						//send(sockfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str()+1, 0));
					}
                }
                break;

              	case EN_MSG_REGISTER:
				{
					string name = root["name"].asString();
					string passwd = root["passwd"].asString();
					string email = root["email"].asString();
		   		   // gUserDBMap[name] = User(name, passwd, email);
		   		   	if (!db.insertIntoUser(name.c_str(), passwd.c_str(), email.c_str()))
					{
						cout<<"do it fail"<<endl;
					}

		      		response["msgtype"] = EN_MSG_ACK;
		       		response["ackcode"] = "yes";
					cout<<"must do it"<<endl;
		        	send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
				cout<<"register send :"<<response.toStyledString().c_str()<<endl;
				}     
	      		break;

	       		case EN_MSG_OFFLINE:
				{
					close(clientfd);
					cout<<"one client offlien"<<endl;
				}
				break;
            }
        }
        //else{}
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
    
    //创建reactor   统一事件源  socket I/O，信号，定时器
    struct event_base* base = event_init();   
    //创建event事件
    struct event *listen_event = event_new(base, listenfd,  EV_READ|EV_PERSIST, ProcListenfd, NULL);
    //把event事件添加到reactor中
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    //启动反应堆
    event_base_dispatch(base);
    //释放事件资源
    event_free(listen_event);
    //关闭reactor
    event_base_free(base);
    
    return 0;
}

/*
class User
{
public:
    User(){}
    User(string name, string pwd, string call)
        :_name(name), _pwd(pwd), _call(call){}
    string getName(){return _name;}
    string getPwd(){return _pwd;}
private:
    string _name;
    string _pwd;
    string _call;
};
map<string, User> gUserDBMap;
map<string, int> chat;
*/
