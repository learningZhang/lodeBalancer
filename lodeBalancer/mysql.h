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
		bool deleteElemBySocket(int fd, int tag);  //һ��ʱ�����ֻ��һ������ʹ�ø�fd
	  //��������Ϣ���д洢��֮�����������в���

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

