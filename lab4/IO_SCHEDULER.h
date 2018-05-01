#pragma once
#include "IO_OPERATION.h"
#include<list>
#include<iostream>
#include <stdlib.h>

using namespace std;

class IO_SCHEDULER
{
public:
	list<IO_OPERATION*> waiting_list;
	IO_SCHEDULER();
	~IO_SCHEDULER();
	virtual bool isEmpty();
	virtual void print_f(IO_OPERATION* operation, int cur_time);
	virtual IO_OPERATION* get_next_operation(int cur_track);
	virtual void add_iooperation(IO_OPERATION* operation);
};

class FIFO_SCHEDULER : public IO_SCHEDULER {
public:
	FIFO_SCHEDULER();
	IO_OPERATION* get_next_operation(int cur_track);
};

class SSTF_SCHEDULER : public IO_SCHEDULER {
public:
	SSTF_SCHEDULER();
	IO_OPERATION* get_next_operation(int cur_track);
};

class LOOK_SCHEDULER : public IO_SCHEDULER {
public:
	bool direction;
	LOOK_SCHEDULER();
	IO_OPERATION* get_next_operation(int cur_track);
};

class CLOOK_SCHEDULER : public IO_SCHEDULER {
public:
	CLOOK_SCHEDULER();
	IO_OPERATION* get_next_operation(int cur_track);
};

class FLOOK_SCHEDULER : public IO_SCHEDULER {
public:
	list<IO_OPERATION*> io_queue[2];
	int active;
	bool direction;
	FLOOK_SCHEDULER();
	bool isEmpty();
	void print_f(IO_OPERATION* operation, int cur_time);
	IO_OPERATION* get_next_operation(int cur_track);
	void add_iooperation(IO_OPERATION* operation);
};
