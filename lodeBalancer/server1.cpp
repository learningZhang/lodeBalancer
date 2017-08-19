#include "head.h"
#include "mysql.h"
#include "server.h"
//#include "memcached.h"

int main(int argc, char **argv)//¸ºÔØ¾ùºâÆ÷
{
	if (argc < 3)
	{
		cout<<"need ip and port";
		return 0;
	} 
    char *ip = argv[1];
    int port = atoi(argv[2]);

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
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

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
    sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (sockaddr*)&client, &len);
    cout<<"new client connect server! client info:"<<inet_ntoa(client.sin_addr)<<" "<<ntohs(client.sin_port)<<endl;

    pthread_t tid;
    pthread_create(&tid, NULL, ReadThread, (void*)clientfd);
}

void* ReadThread(void *arg)
{
	CMysql db;//Ã¿Ò»¸öÏß³ÌÖĞ¿ªÒ»¸ö
    	int clientfd = (int)arg;
    	while(true)
    	{
        	int size = 0;
      		char recvbuf[1024]={0};

        	Json::Reader reader;
       	 	Json::Value root;
        	Json::Value response;

                size = recv(clientfd, recvbuf, 1024, 0);
                if (size == 0)
    	    	{
                	cout<<"client connect fail!"<<errno<<endl;
                	close(clientfd);
                	return NULL;
        	}

		else if (size < 0)//¿¿¿¿¿¿¿¿¿¿¿¿++++++++++
		{
			cout<<"the data server read fail"<<endl;
			continue;
		}

                cout<<"recvbuf is  "<<recvbuf<<endl;
                if (reader.parse(recvbuf, root))
                {
                	int msgtype = root["msgtype"].asInt();
        	    	switch(msgtype)
            		{
                		case EN_MSG_LOGIN:
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
					sendMesgFromDb(db, root["FD"].asInt(), root["name"].asString().c_str(), clientfd); //user logging       
				}
                		break;

                		case EN_MSG_CHAT:
                		{
                                        int tempfd = db.getStates(root["to"].asString().c_str());//¿¿¿¿¿¿¿¿¿¿¿¿¿
                                        cout<<"to "<<root["to"].asString().c_str()<<"  tempfd is  "<<tempfd<<endl;

                                        if(tempfd != -1)
                                        {
                                                response["msgtype"] = EN_MSG_CHAT;
                                                response["from"] = root["from"].asString();
                                                response["msg"] = root["msg"].asString();
                                                response["FD"] = tempfd;
						cout<<"hello "<<endl;
                                        }
                                        else 
                                        {
                                                if (insertIntoMessage(root["from"].asString().c_str(), root["to"].asString().c_str(), root["msg"].asString().c_str(), db) == true)
						{
							cout<<"store is ok"<<endl;
						}
						else
						{
							cout<<"store error"<<endl;
						}
                                                response["FD"] = root["FD"].asInt();
                                                response["msgtype"] = EN_MSG_ACK;
                                                response["ackcode"] = "he isn't inline";
                                        }
                                        send(clientfd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str())+1,0);
                		}
                		break;

                		case EN_MSG_REGISTER:
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

                        	case EN_MSG_OFFLINE:
                                {
                                        close(clientfd);
                                        delStateByfd(db, clientfd);//bool alter_state(int fd);
                                        cout<<"server offlien"<<endl;
                                }
                                break;
            }
        }
    }
}

bool sendMesgFromDb(CMysql &db, int tofd, const char*name, int fd)
{
        Json::Value response;
        char *buff;
        
	char *temp=(char *)malloc(MESSAGE_MAX_LENGTH*sizeof(char));
        if (temp == NULL) return false;
	memset(temp, 0, MESSAGE_MAX_LENGTH);

        if (findMesgByName(db, name, temp, MESSAGE_MAX_LENGTH) == false)
        {
                free(temp);
		temp = NULL;
                return true;
        }
        else
        {
		delSendedMsg(db, name);
                const char *tmp = "-";
                char *fromuser = strtok(buff, tmp);
                char *msg = strtok(NULL, tmp);
		
                response["FD"] = tofd;
                response["msgtype"] = EN_MSG_ACK;
                response["msg"] = "niaho";//buff;
                response["from"] = "zhognguo";//fromuser;

                int x = send(fd, response.toStyledString().c_str(), strlen(response.toStyledString().c_str()), 0);
                if (x == -1)
                {
                        return false;
                }
                free(temp);
		temp = NULL;
                return true;
        }
}
