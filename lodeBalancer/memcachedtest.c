#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmemcached/memcached.h>

int main(int argc, char *argv[]) 
{
	memcached_st *memc;
	memcached_return rc;
	memcached_server_st *servers;
	char value[8191];
	
	//connect multi server连接
	memc = memcached_create(NULL);//创建memcached缓存，类似文件描述符标明此memcached

	servers = memcached_server_list_append(NULL, "localhost", 11211, &rc);
	servers = memcached_server_list_append(servers, "localhost", 11212, &rc);
	rc = memcached_server_push(memc, servers);//追加两个memcached服务器

	memcached_server_free(servers);//++
	
	//Save multi data数据的存储
	size_t i;
	char*keys[]= {"key1", "key2", "key3"};
	size_t key_length[]= {4, 4, 4};//关键字的长度

	char*values[] = {"This is c first value", "This is c second value", "This is c third value"};
	size_t val_length[]= {21, 22, 21};//数据的长度

	for (i=0; i <3; i++)
	{
		//数据的存储
		rc = memcached_set(memc, keys[i],key_length[i], values[i], 
											val_length[i], (time_t)180, (uint32_t)0);
		if (rc == MEMCACHED_SUCCESS) 
		{
			printf("Save key:%s data:\"%s\" success.\n",keys[i], values[i]);
		}
	}
	
	//Fetch multi data获取数据
	char return_key[MEMCACHED_MAX_KEY];
	size_t return_key_length;
	
	char *return_value;
	size_t return_value_length;
	
	uint32_t flags;
	rc = memcached_mget(memc, keys, key_length, 3);//是否类似与mysql的数据存取

			//分布式的存储，多个memcached服务器，如果选择其中的一个进行数据的存储读取	
	while (return_value = memcached_fetch(memc, return_key, &return_key_length, &return_value_length, &flags, &rc)) 
	{          
		if (rc == MEMCACHED_SUCCESS) 
		{
			printf("Fetch key:%s data:%s\n", return_key, return_value);
		}
	}
	
	//Delete multi data删除数据
	for (i=0; i <3; i++) 
	{
		rc = memcached_set(memc, keys[i], key_length[i], values[i], val_length[i], (time_t)180, (uint32_t)0);
		rc = memcached_delete(memc, keys[i], key_length[i], (time_t)0);
		if (rc == MEMCACHED_SUCCESS) 
		{
			printf("Delete %s success\n", keys[i], values[i]);
		}
	}
	//free释放缓存
	memcached_free(memc);
	return 0;
}

//首先，这里面的数据常驻于内存

//memcached可以实现分布式的数据存储，许多个memcached程序开启，客户端想要存储数据，
//利用一致性哈希根据key值来选择一memcached进行数据存储，就是说一个客户端可以连接多个
//memcached
//memcached作为一个服务器，也可与多个客户端相连接

//memcached作为一个服务端，而我是客户端，我与其进行通信，通讯的
//底层其给我封装了，现在的我只需要对其封装后的进行使用