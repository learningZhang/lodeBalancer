#include "head.h"
#include <mysql/mysql.h>


CMysql::CMysql()
{
	ip = "127.0.0.1";
	port = 3306;
	user = "root";
	passwd = "xuanyu";
	pcon = mysql_init((MYSQL*)0);
	if (mysql_real_connect(pcon, ip, user, passwd, NULL, port, NULL, 0) == NULL)
	{
		cout<<"error"<<endl;
		exit(-1);
    }
    
	if(mysql_select_db(pcon, "chat"))
	{
		cout<<"select database error!"<<endl;
		exit(-1);
	}
}

bool CMysql::insertInto_serverfd(int fd, int id)
{
	char sql[100];
	sprintf(sql, "insert into serverfd values('%s', '%s', '%s')", id, fd);
	if (!mysql_real_query(pcon, sql, strlen(sql)))
	{
		return true;
	}
	return false;
}


bool CMysql::get_fd(int id)
{
	char sql[100];
	memset(sql, 0, 100);
	sprintf(sql, "select fd from serverfd where id='%s'", id);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		mysql_error(pcon);	
		cout<<"error in getstate"<<endl;
	}

	pres = mysql_store_result(pcon);
	row = mysql_fetch_row(pres);
	for(int i=0; i<mysql_num_fields(pres); ++i)
	{
		int id = 0;
	 	id = atoi(row[i]);
		return id;
	}
	return -1;
}

bool CMysql::get_id(int fd)
{
	char sql[100];
	memset(sql, 0, 100);
	sprintf(sql, "select id from serverfd where fd='%s'", fd);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		mysql_error(pcon);	
		cout<<"error in getstate"<<endl;
	}

	pres = mysql_store_result(pcon);
	row = mysql_fetch_row(pres);
	for(int i=0; i<mysql_num_fields(pres); ++i)
	{
		int fd = 0;
	 	fd = atoi(row[i]);
		return fd;
	}
	return -1;	
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
	mysql_real_query(pcon, sql, strlen(sql));
	pres = mysql_store_result(pcon);//将结果保存于pres中

	while(row = mysql_fetch_row(pres))//array or false
	{
		for(int i=0; i<mysql_num_fields(pres); ++i)
		{
			if(strcmp(row[1],passwd)==0)
			{
				return true;
			}
		}
	}
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
	memset(sql, 0, 100);
	sprintf(sql, "select socket from state where name='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		mysql_error(pcon);	
		cout<<"error in getstate"<<endl;
	}

	pres = mysql_store_result(pcon);
	row = mysql_fetch_row(pres);
	for(int i=0; i<mysql_num_fields(pres); ++i)
	{
		int fd = 0;
	 	fd = atoi(row[i]);
		return fd;
	}
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
