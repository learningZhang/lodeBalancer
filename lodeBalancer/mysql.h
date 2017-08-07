#include <iostream>
using namespace std;
#include <mysql/mysql.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

class CMysql
{
	public:
		CMysql();
		//for user
		bool insertIntoUser(const char *name, const char *passwd, const char *call);
		bool queryPasswd(const char *name, const char *passwd);
		bool deleteUser(const char *name);

		//for state
		int getStates(const char *name);
		bool insertIntoStates(const char *name, int fd);		//bool insertInto_serverfd(int fd, int id);
		bool deleteElemBySocket(int fd, int tag);  //一个时间段中只有一个进程使用该fd
	  //对离线信息进行存储，之后按照姓名进行查找

		//for message
		bool insertIntoMessage(const char*from, const char*to, const char *mesg);//+time
		bool delInMessage(const char *name);
		bool findMessageByName(const char*name,char*str,int length);
	private:
		MYSQL *pcon;
	 	MYSQL_RES *pres;
		MYSQL_ROW row;
		
	 	const char *ip;
		const char *user;
		const char *passwd;
		unsigned short port;
};
/*
class Cmysql
{
	public:
		static CMysql* createMysql()
		{
			if (mysql == NULL)
			{
			///	lock()
			//	{
					if (mysql == NULL)
					{
						mysql = new CMysql;
					}
			//	}
			}
			return mysql;
		}
		static CMysql *mysql;	
	private:
		Cmysql()
		{
			CMysql();
		}
		~Cmysql()
		{
			delete mysql;
		}
		//pthread_mutex_t mutex;//对mutex进行初始化
};
CMysql *Cmysql::mysql=NULL;
*/
