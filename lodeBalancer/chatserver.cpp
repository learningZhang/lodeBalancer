#include <iostream>
#include <string>
#include <map>
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

    int clientfd = (int)arg;
    while(true)
    {//����client���͵���Ϣ��������Ӧ
        int size = 0;
        char recvbuf[1024]={0};
        Json::Reader reader;
        Json::Value root;
        Json::Value response;
	
	size = recv(clientfd, recvbuf, 1024, 0);
	if(size <= 0)//when a client closed,the size is 0
        {
            cout<<"errno:"<<errno<<endl;
            cout<<"client connect fail!"<<endl;
            close(clientfd);
            return NULL;
        }
    
        cout<<"recvbuf:"<<recvbuf<<endl;
        
        if(reader.parse(recvbuf, root))//�жϵ���ʲô
        {
            int msgtype = root["msgtype"].asInt();
            switch(msgtype)
            {
                case EN_MSG_LOGIN:
                {
                    response["msgtype"] = EN_MSG_ACK;
                    string name = root["name"].asString();
                    string pwd = root["pwd"].asString();
                    
                    map<string, User>::iterator it = gUserDBMap.find(name);
                    if(it == gUserDBMap.end())
                    {
                        response["ackcode"] = "error";
                    }
                    else
                    {
                        if(pwd == it->second.getPwd())
                        {
                            response["ackcode"] = "ok";
                            chat[name.c_str()] = clientfd;//�û���½�ɹ�֮ʱ��¼��ֵ��<name, fd>
                            cout<<"name"<<name<<"  fd"<<clientfd<<endl;
                        }
                        else
                        {
                            response["ackcode"] = "error";
                        }
                    }
                    cout<<"response:"<<response.toStyledString()<<endl;
                    send(clientfd, response.toStyledString().c_str(),
                        strlen(response.toStyledString().c_str())+1, 0);
                }
                break;
                
                case EN_MSG_CHAT:
                {
                    cout<<root["from"]<<" => "<<root["to"]<<":"<<root["msg"]<<endl;    
                   	map<string, int>::iterator it = chat.find(root["to"].asString());
					if(it != chat.end())
					{
						response["msgtype"] = EN_MSG_CHAT;
						response["from"] = root["from"].asString();
						response["msg"] = root["msg"].asString();
						send(it->second, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0);
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
		   		    gUserDBMap[name] = User(name, passwd, email);
		      		response["msgtype"] = EN_MSG_ACK;
		       		response["ackcode"] = "yes";
		        	send(clientfd, response.toStyledString().c_str(),strlen(response.toStyledString().c_str())+1, 0);
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
    
    gUserDBMap["zhang"] = User("zhang", "111", "9870986796");
    gUserDBMap["xiao"] = User("xiao", "222", "7777777");
    gUserDBMap["gao yang"] = User("gao yang", "333333", "66666666");
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        cout<<"listenfd create fail!"<<endl;
        return -1;
    }
    
    sockaddr_in server;
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
    
    //����reactor   ͳһ�¼�Դ  socket I/O���źţ���ʱ��
    struct event_base* base = event_init();   
    //����event�¼�
    struct event *listen_event = event_new(base, listenfd,  EV_READ|EV_PERSIST, ProcListenfd, NULL);
    //��event�¼���ӵ�reactor��
    event_add( listen_event, NULL );
    
    cout<<"server started..."<<endl;
    //������Ӧ��
    event_base_dispatch(base);
    //�ͷ��¼���Դ
    event_free(listen_event);
    //�ر�reactor
    event_base_free(base);
    
    return 0;
}
