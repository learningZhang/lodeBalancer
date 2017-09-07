#include <iostream>
using namespace std;
#include <event.h>

void timeout_cb(int fd, short event, void *argc)
{
	cout<<"timeout"<<endl;
}
int main()
{
	struct event_base *base = event_init();
		
	timeval tv = {6,5};
	timeval tv2 = {6,4};

	struct event*timeout_event = evtimer_new(base, timeout_cb, NULL);
	event_add(timeout_event, &tv);

	struct event*timeout_even = evtimer_new(base, timeout_cb, NULL);
	event_add(timeout_even, &tv2);
	
	event_base_dispatch(base);
	event_free(timeout_event);
	event_base_free(base);
	return 0;
}
