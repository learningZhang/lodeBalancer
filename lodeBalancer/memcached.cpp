#include "memcached.h"

CMemcached::CMemcached(char *ip, unsigned short port):memc(NULL), server(NULL)
{
    memc = memcached_create(NULL);//进行创建
    if(memc == NULL)
    {
        cout<<"memcached create error"<<endl;
        exit(-1);
    }
    
    server = memcached_server_list_append(NULL, ip, port, &rc);
    if(server == NULL)
    {
        cout<<"memcached create server error"<<endl;
        exit(-1);
    }
   
    rc = memcached_server_push(memc, server);
    if(MEMCACHED_SUCCESS != rc)
    {
        cout<<"memcached server push error"<<endl;
        exit(-1);
    }
}

CMemcached::~CMemcached()
{
    memcached_server_list_free(server);
    memcached_free(memc);
}

char* CMemcached::queryKey(char *key)
{
    size_t value_length = 0;
    char* result = memcached_get(memc, key, strlen(key), NULL, NULL, &rc);
    if(rc == MEMCACHED_SUCCESS)
    {
        return result;
    }
    else
    {
        return NULL;
    }
}

void CMemcached::saveValue(char *key, char *value)
{
    rc = memcached_set(memc, key, strlen(key), value, strlen(value), 0, 0);
    if(rc == MEMCACHED_SUCCESS)
    {
        cout<<"memcached key:"<<key<<" value:"<<value<<" success!"<<endl;
    }
}
//memcached对内存的管理
//memcached成功之后可以采用phpstudy来对数据库进行进一步的提升