class CMutex
{
public:
	CMutex();
	~CMutex();
	bool lock();
	bool trylock();
	bool unlock();
private:
	pthread_mutex_t mutex;
};

void get_pipefd_clock(int &disable, int epollfd, int fd, CMutex &mutex);

void getoff_pipefd_clock(int epollfd, int fd, CMutex &mutex);

int addEvent(int , int);
int deleteEvent(int ,int);
