#include "head.h"
#include "threadPool.h"

pthread_t *CPthreadPool::createPthreadPool(int num, void*(*func)(void *))
{
	if (pitArray == NULL)
	{
		CPthreadPool(num, func);
	}
	return pitArray;
}
		
CPthreadPool::~CPthreadPool()
{
	delete [] pitArray;//关闭线程
}

CPthreadPool::CPthreadPool(int n, void*(*fun)(void *)):num(n),func(fun)
{
	pitArray = new pthread_t[num];
	for(int i=0; i<num; i++)
	{
		int res = pthread_create(pitArray+i, NULL, func, NULL);
		assert(res != -1);
		cout<<"pthread["<<i<<"]  open"<<endl;
	}			
}

pthread_t* CPthreadPool::pitArray = NULL;//进行初始化

