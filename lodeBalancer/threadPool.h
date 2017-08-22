class CPthreadPool
{
	public:
		static  pthread_t *createPthreadPool(int num, void*(*func)(void*));
		~CPthreadPool();
	private:
		CPthreadPool(int _num, void*(*fun)(void*));
		void*(*func)(void *);//线程函数指针，返回值是void*，传入参数是void*
		int num;
		static pthread_t *pitArray;
};
