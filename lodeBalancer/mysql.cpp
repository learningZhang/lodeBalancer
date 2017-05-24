<<<<<<< Updated upstream
#include "head.h"
#include <mysql/mysql.h>

=======
#include "mysql.h"
>>>>>>> Stashed changes

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

<<<<<<< Updated upstream
int CMysql::get_fd(int id)
=======
bool CMysql::insertIntoMessage(const char*from, const char*to, const char *mesg)//+time
{
	char sql[1024];
	sprintf(sql, "insert into message values('%s', '%s', '%s')", from, to, mesg);

	if (!mysql_real_query(pcon, sql, strlen(sql)))
	{
		return true;
	}
	return false;
}

bool CMysql::delInMessage(const char *name)
{
	char sql[100];
	sprintf(sql, "delete from message where touser='%s'", name);
	if(!mysql_real_query(pcon, sql, strlen(sql)))
	{
		return true;
	}
	return false;
}

bool CMysql::findMessageByName(const char* name, char *message, int length)
{
	char sql[100];
	sprintf(sql, "select fromuser,msg from message where touser='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{//if success will return 0,if fail no 0
		cout<<mysql_error(pcon)<<endl;		
		return false;
	}

	if ((pres=mysql_store_result(pcon)) != NULL)
	{
		if ((row=mysql_fetch_row(pres)) != NULL)
		{
			int lenth =strlen(row[0]);
			if (length > lenth)
			{
				strcpy(message, row[0]); 
				strcat(message, "-");//é€šè¿‡åˆ†å‰²å·æ¥åˆ†å¼€
				strcat(message, row[1]);
			}
			else
			{
				return false;
			}
	//		char *str = (char *)malloc(lenth+1);//ç”¨å®Œä¹‹åè®°å¾—å†…å­˜é‡Šæ”¾
	//		strcpy(str,row[0]); 
	//		strcat(str,"-");//é€šè¿‡åˆ†å‰²å·æ¥åˆ†å¼€
	//		strcat(str,row[1]);
			return true;
		}
		if (mysql_error(pcon) != NULL)
		{
			cout<<mysql_error(pcon)<<endl;
		}
	}
	mysql_free_result(pres);
	return false;	
}
//ä»åˆ—è¡¨ä¸­è·å¾—è¯¥passwd,ç„¶åä¸passwdè¿›è¡Œæ¯”è¾ƒ
bool CMysql::queryPasswd(const char *name, const char *passwd)
>>>>>>> Stashed changes
{
	char sql[100];
	sprintf(sql, "select fd from serverfd where id=%d", id);
	if (NULL == mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in getstate "<<mysql_error(pcon)<<endl;
		return -2;
	}
<<<<<<< Updated upstream

	if ((pres = mysql_store_result(pcon)) != NULL)//¿¿¿¿¿¿¿
=======
	
	if ((pres = mysql_store_result(pcon)) != NULL)//å°†ç»“æœä¿å­˜äºpresä¸­
>>>>>>> Stashed changes
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿NULL
		{
			int id = atoi(row[0]);
			cout<<id;
			mysql_free_result(pres);
			return id;
		}
		if (mysql_error(pcon))//¿¿¿¿mysql_fetch_row¿¿¿¿¿¿
		{
			cout<<"error in get_id  "<<mysql_error(pcon)<<endl;
		}
	}
	else//mysql_store_result¿¿¿¿¿¿
	{
		cout<<"error in get_fd "<<mysql_error(pcon)<<endl;	
	}
	mysql_free_result(pres);
	return -1;
}

//int res = msyql_num_fileds(pres);¿¿¿¿¿¿¿¿¿¿
int CMysql::get_id(int fd)
{
	char sql[100];
	sprintf(sql, "select id from serverfd where fd=%d ", fd);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<"error in getstate"<<mysql_error(pcon)<<endl;//¿¿¿¿
		return -2;
	}
<<<<<<< Updated upstream

	if ((pres = mysql_store_result(pcon)) != NULL)
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//¿¿¿¿¿¿
=======
        
	if ((pres = mysql_store_result(pcon)) != NULL)//é é é é é ?.é é é é é é  2.é é é é é é ?3.é é é ?
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//é é é é 
>>>>>>> Stashed changes
		{
			int fd = atoi(row[0]);//¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
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

<<<<<<< Updated upstream
bool CMysql::insertIntoUser(const char *name, const char *passwd, const char *call)
{
	char sql[100];
	sprintf(sql, "insert into user values('%s', '%s', '%s')", name, passwd, call);
	if (!mysql_real_query(pcon, sql, strlen(sql)))
=======
bool CMysql::deleteElemBySocket(int fd, int tag)//ä¸€ä¸ªæ—¶é—´æ®µä¸­åªæœ‰ä¸€ä¸ªè¿›ç¨‹ä½¿ç”¨è¯¥fd
{
	char sql[100];
	if (tag == 1)
	{
		sprintf(sql, "delete from state where socket = %d", fd);
	}
	else
	{
		sprintf(sql, "delete from state");
	}
	
	int res = mysql_real_query(pcon, sql, strlen(sql));//æˆåŠŸè¿”å›0
	if (!res)
>>>>>>> Stashed changes
	{
		return true;
	}
	return false;
}

<<<<<<< Updated upstream
//´ÓÁĞ±íÖĞ»ñµÃ¸Ãpasswd,È»ºóÓëpasswd½øĞĞ±È½Ï
bool CMysql::queryPasswd(const char *name, const char *passwd)
{
	char sql[100];
	sprintf(sql, "select name, pwd from user where name='%s'", name);
	if (mysql_real_query(pcon, sql, strlen(sql)))
	{
		cout<<mysql_error(pcon)<<endl;
		return -2;
	}
	
	if ((pres = mysql_store_result(pcon)) != NULL)//½«½á¹û±£´æÓÚpresÖĞ
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
        
	if ((pres = mysql_store_result(pcon)) != NULL)//¿¿¿¿¿¿¿¿¿¿¿1.¿¿¿¿¿¿¿¿¿¿¿¿ 2.¿¿¿¿¿¿¿¿¿¿¿¿¿ 3.¿¿¿¿¿¿¿
	{
		if ((row = mysql_fetch_row(pres)) != NULL)//¿¿¿¿¿¿¿¿
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
=======
>>>>>>> Stashed changes
