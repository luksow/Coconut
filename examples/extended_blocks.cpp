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
	c_begin_block("inv");
	invalidated = true;
	c_end_block();

	c_begin_block("clear");
	while (!requests.empty())
		requests.pop();
	c_end_block();
}

void processFirstRequest()
{
	if (c_cond_block(invalidated || requests.empty(), "check"))
		return;

	c_begin_block("process");
	c_assert_false(requests.empty(), "requests queue is empty!");
	processRequest(requests.front());
	requests.pop();
	c_end_block();
}

int main()
{
	c_init();

	// test1 - should pass
	invalidated = false;
	requests.push(Request());
	std::cout << "Running test1" << std::endl;
	c_set_blocks_interleaving("inv;check;clear;process");
	auto t1 = new std::thread(invalidateRequests);
	auto t2 = new std::thread(processFirstRequest);
	t1->join();
	t2->join();
	delete t1;
	delete t2;

	// test2 - impossible interleaving
	invalidated = false;
	requests.push(Request());
	std::cout << "Running test2" << std::endl;
	c_set_blocks_interleaving("clear;process;inv;check");
	t1 = new std::thread(invalidateRequests);
	t2 = new std::thread(processFirstRequest);
	t1->join();
	t2->join();
	delete t1;
	delete t2;

	// test3 - should fail
	invalidated = false;
	requests.push(Request());
	std::cout << "Running test3" << std::endl;
	c_set_blocks_interleaving("check;inv;clear;process");
	t1 = new std::thread(invalidateRequests);
	t2 = new std::thread(processFirstRequest);
	t1->join();
	t2->join();
	delete t1;
	delete t2;

	c_free();
	return 0;
}
