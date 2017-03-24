#include "head.h"

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
		bool insertIntoStates(const char *name, int fd);
		//bool insertInto_serverfd(int fd, int id);
		bool deleteElemBySocket(int fd);  //一个时间段中只有一个进程使用该fd
		int findFdByName(const char* name);  //对离线信息进行存储，之后按照姓名进行查找

		//for message
		bool insertIntoMessage(const char*from, const char*to, const char *mesg);//+time
		bool delInMessage(cosnt char *name);
		char * findMesgByName(const char*name);
	private:
		MYSQL *pcon;
	 	MYSQL_RES *pres;
		MYSQL_ROW row;
		
	 	const char *ip;
		const char *user;
		const char *passwd;
		unsigned short port;
};

bool insertIntoMessage(const char*from, const char*to, const char *mesg)//+time
{}
bool delInMessage(cosnt char *name)
{}
char* findMesgByName(const char*name)
{}

CMysql::CMysql()
{
	ip = "127.0.0.1";
	port = 3306;
	user = "root";
	passwd = "xuanyu";
	pcon = mysql_init((MYSQL*)0);
	if (mysql_real_connect(pcon, ip, user, passwd, NULL, port, NULL, 0) == NULL)
	{
		cout<<"connect error"<<endl;
		exit(-1);
    }
    
	if(mysql_select_db(pcon, "chat"))
	{
		cout<<"select database error!"<<endl;
		exit(-1);
	}
}

bool CMysql::insertIntoUser(const char *name, const char *passwd, const char *call)
{
	char sql[100];
	sprintf(sql, "insert into user values('%s', '%s', '%s')", name, passwd, call);
	if (!mysql_real_query(pcon, sql, strlen(sql)))
	{
		return true;
	}
	return false;
}

//从列表中获得该passwd,然后与passwd进行比较
bool CMysql::queryPasswd(const char *name, const char *passwd)
{
	char sql[100];
	sprintf(sql, "select name, pwd from user where name='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<mysql_error(pcon)<<endl;
		return -2;
	}
	
	if ((pres = mysql_store_result(pcon)) != NULL)//将结果保存于pres中
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//array or false
		{
			if( strcmp(row[1],passwd) == 0 )
			{
				mysql_free_result(pres);
				return true;
			}
		}
		if (mysql_error(pcon) != NULL)
		{
			cout<<mysql_error(pcon)<<endl;	
		}
	}
	mysql_free_result(pres);
	return false;
}

bool CMysql::deleteUser(const char *name)
{
	char sql[100];
	sprintf(sql, "delete from user where name='%s'", name);
	int res = mysql_real_query(pcon, sql, strlen(sql));
	if (!res)
	{
		return true;
	}
	return false;
}

int CMysql::getStates(const char *name)
{
	char sql[100];
	sprintf(sql, "select socket from state where name='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in getstate "<<mysql_error(pcon)<<endl;
		return -2;
	}
        
	if ((pres = mysql_store_result(pcon)) != NULL)//靠靠靠靠靠�1.靠靠靠靠靠靠 2.靠靠靠靠靠靠� 3.靠靠靠�
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//靠靠靠靠
		{
			int re = atoi(row[0]);
			mysql_free_result(pres);
			return re;
		}

		if (mysql_error(pcon) != NULL)
		{
			cout<<"error in getstates "<<mysql_error(pcon)<<endl;
		}
	}
	else
	{
		cout<<"error in getStates "<<mysql_error(pcon)<<endl;
		return -1; 
	}
	mysql_free_result(pres);
	return -1;
}

bool CMysql::insertIntoStates(const char *name, int fd)
{
	char sql[100];
	sprintf(sql, "insert into state values('%s', '%d')", name, fd);
	int res = mysql_real_query(pcon, sql, strlen(sql));
	if (!res)
	{
		return true;
	}
	return false;
}

bool CMysql::deleteElemBySocket(int fd, int teg)//一个时间段中只有一个进程使用该fd
{
	char sql[100];
	if (tag == 1)
	{
		sprintf(sql, "delete from state where fd = %d", fd);
	}
	else
	{
		sprintf(sql, "delete from state");
	}
	
	int res = mysql_real_query(pcon, sql, strlen(sql));//成功返回0
	if (!res)
	{
		return true;
	}
	return false;
}

//致命--两个人的名字相同，设置id
int CMysql::findFdByName(const char *name)//对离线信息进行存储，之后按照姓名进行查找
{
	char sql[100];
	sprintf(sql, "select fd from state where name='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in findFdByName"<<endl;
		cout<<mysql_error(pcon)<<endl;
		return -1;
	}
	if ((pres = mysql_store_result(pcon)) != NULL)
	{
		if ((row = mysql_fetch_row) != NULL)
		{
			int re = row[0];
			mysql_free_result(pres);
			return re;
		}
		if (mysql_error(pcon)!=NULL)
		{
			cout<<mysql_error(pcon)<<endl;
		}
	} 
	mysql_free_result(pres);
	return false;
}


/*
bool CMysql::insertInto_serverfd(int id, int fd)
{
	char sql[100];
	sprintf(sql, "insert into serverfd values('%d','%d')", id, fd);
	if (!mysql_real_query(pcon, sql, strlen(sql)))
	{
		return true;
	}
	return false;
}

int CMysql::get_fd(int id)
{
	char sql[100];
	sprintf(sql, "select fd from serverfd where id=%d", id);
	if (NULL == mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in getstate "<<mysql_error(pcon)<<endl;
		return -2;
	}

	if ((pres = mysql_store_result(pcon)) != NULL)//靠靠靠?
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//靠靠靠靠靠靠靠靠靠靠靠靠靠NULL
		{
			int id = atoi(row[0]);
			cout<<id;
			mysql_free_result(pres);
			return id;
		}
		if (mysql_error(pcon))//靠靠mysql_fetch_row靠靠靠
		{
			cout<<"error in get_id  "<<mysql_error(pcon)<<endl;
		}
	}
	else//mysql_store_result靠靠靠
	{
		cout<<"error in get_fd "<<mysql_error(pcon)<<endl;	
	}
	mysql_free_result(pres);
	return -1;
}

//int res = msyql_num_fileds(pres);靠靠靠靠靠
int CMysql::get_id(int fd)
{
	char sql[100];
	sprintf(sql, "select id from serverfd where fd=%d ", fd);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in getstate"<<mysql_error(pcon)<<endl;//靠靠
		return -2;
	}

	if ((pres = mysql_store_result(pcon)) != NULL)
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//靠靠靠
		{
			int fd = atoi(row[0]);//靠靠靠靠靠靠靠靠靠靠
			cout<<"fd is"<<fd<<endl;
			return fd;
		}
		if (mysql_error(pcon) != NULL)
		{
			cout<<"error "<<mysql_error(pcon)<<endl;	
		}
	}
	else
	{
		cout<<"in get_id "<<mysql_error(pcon);
	}
	mysql_free_result(pres);
	return -1;	
}
*/