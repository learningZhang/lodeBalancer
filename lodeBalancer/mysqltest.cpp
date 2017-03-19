#include <iostream>
#include <string>
using namespace std;
#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>

class CUser
{
	public:        
	private:
        char name[20];
        char pwd[20];
};

int main()
{
	MYSQL* pcon;
	MYSQL_RES *pres;
	MYSQL_ROW row;

	pcon = mysql_init((MYSQL*)0);
	MYSQL *tmp = mysql_real_connect(pcon, "127.0.0.1", "root", "xuanyu", NULL, 5000, NULL, 0);
	if (tmp == NULL)
	{
		cout<<"error"<<endl;
	}
	cout<<"connect "<<endl;
	mysql_select_db(pcon, "chat");

	char sql[100];
	sprintf(sql, "select * from user");
	mysql_real_query(pcon, sql, strlen(sql));

	pres = mysql_store_result(pcon);
	while(row = mysql_fetch_row(pres))
	{
		for(int i=0; i<mysql_num_fields(pres); ++i)
		{
			cout<<row[i]<<" ";
		}
		cout<<endl;
	}
	mysql_free_result(pres);
	mysql_close(pcon);

	return 0;
}
