#include <iostream>
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
#include <event.h>
#include <json/json.h>

typedef enum _MsgType
{
    EN_MSG_LOGIN=1,
    EN_MSG_REGISTER,
    EN_MSG_CHAT,
    EN_MSG_OFFLINE,
    EN_MSG_ACK
}EnMsgType;

//全局记录用户名和密码
char name[20];
char pwd[20];


//异步接收读线程
void* ReadThread(void *arg)
{
    int clientfd = *(int*)arg;
    char recvbuf[1024];
    Json::Reader reader;
    Json::Value root;
    int size=0;

    while(true)
    {
        size = recv(clientfd, recvbuf, 1024, 0);
        if(size < 0)
        {
            cout<<"server connect fail!"<<endl;
            break;
        }
        if(reader.parse(recvbuf, root))
        {
            int msgtype = root["msgtype"].asInt();
            switch(msgtype)
            {
                case EN_MSG_CHAT:
                {
                    cout<<root["from"].asString()<<":"<<root["msg"]<<endl;
                }
                break;
                case EN_MSG_ACK:
	            {
		            cout<<root["ackcode"].asString()<<endl;
	            }
	            break;
            }
        }
    }
}

bool registe(int fd)//注册
{
	char email[20];
	char passwd[20];
	cout<<"your name: ";
	cin.getline(name, 20);
	cout<<"input passwd: ";
	cin.getline(passwd, 20);
	cout<<"your emial: ";
	cin.getline(email, 20);
	
	Json::Value root;
	root["msgtype"] = EN_MSG_REGISTER;
	root["name"] = name;
	root["passwd"] = passwd;
	root["email"] = email;
 	char recvbuf[1024];
	int size = send(fd, root.toStyledString().c_str(),
	 	strlen(root.toStyledString().c_str()), 0);
	if (size < 0)
	{
		cout<<"register info send fail"<<endl;
		exit(0);
	}

	char recvbuff[1024];
	size = recv(fd, recvbuf, 1024, 0);
	if (size < 0)
	{
		cout<<"register info ack fail"<<endl;
	}
	 //检查返回信息如果正确就退到登陆界面
	Json::Reader reader;
	if(reader.parse(recvbuf, root))
	{
        	int msgtype = root["msgtype"].asInt();
       		if(msgtype != EN_MSG_ACK)
        	{
            		cout<<"recv server login ack msg invalid!"<<endl;
            		exit(0);
        	}
       		 string ackcode = root["ackcode"].asString();
        	if(ackcode == "yes")
        	{
            		return true;
        	}
       		return false;
    }
    return false;
}

bool doLogin(int fd)//登陆
{
    cout<<"name:";
    cin.getline(name, 20);
    cout<<"pwd:";
    cin.getline(pwd, 20);
    
    Json::Value root;
    root["msgtype"] = EN_MSG_LOGIN;
    root["name"] = name;
    root["pwd"] = pwd;

    cout<<"my fd is"<<fd<<endl;    
    int size = send(fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);
    if(size < 0)
    {
        cout<<"send login msg fail!"<<endl;
        exit(0);
    }
    
    char recvbuf[1024]={0};
    size = recv(fd, recvbuf, 1024, 0);
    if(size <= 0)
    {
        cout<<"recv server login ack fail!"<<endl;
        exit(0);
    }
    
    Json::Reader reader;
    if(reader.parse(recvbuf, root))
    {
        int msgtype = root["msgtype"].asInt();
        if(msgtype != EN_MSG_ACK)
        {
            cout<<"recv server login ack msg invalid!"<<endl;
            exit(0);
        }
        string ackcode = root["ackcode"].asString();
        if(ackcode == "ok")
        {
            return true;
        }
        return false;
    }
    return false;
}

bool offline(int fd)
{
	Json::Value  root;
	root["msgtype"] = EN_MSG_OFFLINE;
	int size = send(fd, root.toStyledString().c_str(), 
			strlen(root.toStyledString().c_str()), 0);
	if (size < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// 命令行参数传入服务器port
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cout<<"input ./client port"<<endl;
        return -1;
    }
    int port=0;
    port = atoi(argv[1]);
    
    int clientfd;
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        cout<<"clientfd create fail!"<<endl;
        return -1;
    }
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(-1 == connect(clientfd, (sockaddr*)&server, sizeof(server)))
    {
        cout<<"connect server fail!"<<endl;
        return -1;
    }
   
    int choice = 0;
    bool bloginsuccess = false;
    while(!bloginsuccess)
    {
        cout<<"============"<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.register"<<endl;
        cout<<"3.exit"<<endl;
        cout<<"============"<<endl;   //异常退出-信号
        cout<<"choice:";
        cin>>choice;
        cin.get();
        
        switch(choice)
        {
            case 1://login
            	if(doLogin(clientfd))    { bloginsuccess = true;}  
            	else{cout<<"login fail!name or pwd is wrong!"<<endl;}  
            	break;
            
            case 2:
           	 if (registe(clientfd))    {cout<<"register successful!"<<endl;}
           	 else {cout<<"register failed!"<<endl;}
		 continue;
            case 3:
		offline(clientfd);
		close(clientfd);
	        cout<<"bye bye..."<<endl;
		exit(0);
            default:
                cout<<"invalid input!"<<endl;
		fflush(stdin);
		continue;
        }
    }
    cout<<"welcome to chat system!"<<endl;
    pthread_t tid;
    pthread_create(&tid, NULL, ReadThread, &clientfd);
    
    int size=0;
    while(true)
    {
        char chatbuf[1024] = {0};
        cin.getline(chatbuf, 1024);
        
        if(strcmp(chatbuf, "quit") == 0)
        {
	        offline(clientfd);
	        cout<<"by close to shutdown client"<<endl;
		close(clientfd);
                exit(0);
        }
        
        string parsestr = chatbuf;
        int offset = parsestr.find(':');
        Json::Value root;
        root["msgtype"] = EN_MSG_CHAT;
        root["from"] = name;
        root["to"] = parsestr.substr(0, offset);
        root["msg"] = parsestr.substr(offset+1, parsestr.length()-offset-1);
        size = send(clientfd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str())+1, 0);
    }
    return 0;
}
