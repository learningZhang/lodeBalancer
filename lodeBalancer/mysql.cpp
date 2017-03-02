#include <iostream>
#include <string>
using namespace std;
#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>

class CMysql
{
	public:
		CMysql();
		int insertIntoUser(char *name, char *passwd, char *call);
		bool queryPasswd(char *name, char *passwd);
		bool deleteUser(char *name);
		int getStates(char *name);
		int insertIntoStates(char *name, int fd);
	private:
		MYSQL *pcon;
	 	MYSQL_RES *pres;
		MYSQL_ROW row;
		
		char *ip;
		char *user;
		char *passwd;
		unsigned short port;
};

int main()
{
	CMysql sql;
	char* name = "zhanghao";
	char* passwd = "11122";
	char *call = "110110110";
	sql.insertIntoUser(name, passwd, call);
	return 0;
}

CMysql::CMysql()
{
	ip = "127.0.0.1";
	port = 3306;
	user = "root";
	passwd = "xuanyu";
	pcon = mysql_init((MYSQL*)0);
	MYSQL* tmp = mysql_real_connect(pcon, ip, user, passwd, NULL, port, NULL, 0);
	if (tmp == NULL)
	{
		cout<<"error"<<endl;
    }
	mysql_select_db(pcon, "chat");
}

int CMysql::insertIntoUser(char *name, char *passwd, char *call)
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
bool CMysql::queryPasswd(char *name, char *passwd)
{
	char sql[100];
	sprintf(sql, "select pwd from user where name='%s';", name);
	mysql_real_query(pcon, sql, strlen(sql));
	pres = mysql_store_result(pcon);//将结果保存于pres中
	row = mysql_fetch_row(pres);

	for(int i=0; i<mysql_num_fields(pres);i++)
	{
		cout<<row[i]<<endl;
	}
}

bool CMysql::deleteUser(char *name)
{
	char sql[100];
	sprintf(sql, "delete from user where name='%s';", name);
}

int CMysql::getStates(char *name)
{
	char sql[100];
	sprintf(sql, "select fd from state where name='%s';", name);
	mysql_real_query(pcon, sql, strlen(sql));
	pres = mysql_store_result(pcon);
	
	row = mysql_fetch_row(pres);
	for(int i=0; i<mysql_num_fields(pres); ++i)
	{
		cout<<row[i]<<endl;
	}
	cout<<"-------------"<<endl;
}

int CMysql::insertIntoStates(char *name, int fd)
{
	char sql[100];
	sprintf(sql, "insert into state values('%s', '%d');", name, fd);
	int res = mysql_real_query(pcon, sql, strlen(sql));
	if (!res)
	{
		return true;
	}
	return false;
}
