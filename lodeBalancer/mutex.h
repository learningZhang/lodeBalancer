
class　CMutex
{
public:
	CMutex();
	~CMutex();
private:
	bool lock();
	bool trylock();
	bool unlock();
};

void get_pipefd_clock(int &disable, int epollfd, int fd, CMutex &mutex);

void getoff_pipefd_clock(int epollfd, int fd, CMutex &mutex);