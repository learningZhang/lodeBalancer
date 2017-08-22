#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "mutex.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
CMutex::CMutex()
{
	if (pthread_mutex_init(&mutex, NULL)==0)
	{
		return ;
	}
	else
	{
		cout<<"error"<<endl;
		exit(-1);
	}
}

CMutex::~CMutex()
{
	pthread_mutex_destroy(&mutex);
}

bool CMutex::lock()
{
	return pthread_mutex_lock(&mutex)==0;
}

bool CMutex::trylock()
{
	return pthread_mutex_trylock(&mutex)==0;	
}

bool CMutex::unlock()
{
	return pthread_mutex_unlock(&mutex);
}


void get_pipefd_clock(int &disable, int epollfd, int fd, CMutex &mutex)
{
	if (disable > 0)
	{
		--disable;
	}
	else
	{
		if(mutex.trylock() == false)//尝试去得到锁,失败
		{
			return ;
		}
		else
		{
			addEvent(epollfd, fd);	//将文件描述符台添加到epoll中
		}
	}
}

void getoff_pipefd_clock(int epollfd, int fd, CMutex &mutex)
{
	if (mutex.unlock()== true)
	{
		deleteEvent(epollfd, fd);//将管道文件描述符从epoll中去除
	}
	else
	{
		cout<<"error"<<endl;
	}
}

/*
#include <semaphore.h>
class sem
{
public:
	sem()
	{
		assert(sem_init(&m_sem, 0, 0)!=0);
	}
	~sem()
	{
		sem_destroy(&m_sem);	
	}
	bool wait()
	{
		return sem_trywait(&m_sem)==0;//如果信号量大于0，则减1操作，否则返回-1
	}
	bool post()
	{
		return sem_post(&m_sem)==0;
	}
private:
	sem_t m_sem;
};

*/
