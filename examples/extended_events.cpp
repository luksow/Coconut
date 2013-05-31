#include <thread>
#include <queue>
#include <iostream>

#include "coconut.h"

struct Request
{
};

bool invalidated;

std::queue<Request> requests;

void processRequest(Request& r)
{
}

void invalidateRequests()
{
	c_wait_event("checked");
	invalidated = true;

	while (!requests.empty())
		requests.pop();
	c_publish_event("cleared");
}

void processFirstRequest()
{
	if (invalidated || requests.empty())
		return;
	c_publish_event("checked");

	c_wait_event("cleared");
	c_assert_false(requests.empty(), "requests queue is empty!");
	processRequest(requests.front());
	requests.pop();
}

int main()
{
	c_init();

	invalidated = false;
	requests.push(Request());
	auto t1 = new std::thread(invalidateRequests);
	auto t2 = new std::thread(processFirstRequest);
	t1->join();
	t2->join();
	delete t1;
	delete t2;

	c_free();
	return 0;
}
