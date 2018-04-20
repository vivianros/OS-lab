#pragma once

#include "Process.h"
#include <list>
using namespace std;

class Scheduler
{

public:
	list<Process*> readyList;
	int quantum;
	int PR; // Process Running time
	int IU; // IO Utilization (time processes performing IO)
	int FT;

	Scheduler(int q);
	~Scheduler();
	int get_quantum();
	int get_FT();
	void set_FT(int time);
	void set_quantum(int q);

	virtual void add_process(Process* proc);
	virtual Process* get_next_process();
};

class FCFS_Scheduler : public Scheduler {
public:
	FCFS_Scheduler(int q);

};

class LCFS_Scheduler : public Scheduler {
public:
	LCFS_Scheduler(int q);
	virtual Process* get_next_process();

};

class SJF_Scheduler : public Scheduler {
public:
	SJF_Scheduler(int q);

	virtual Process* get_next_process();
};

class Prio_Scheduler : public Scheduler {
public:
	list<Process*> activeList[4];
	list<Process*> expiredList[4];
	Prio_Scheduler(int q);
	virtual void add_process(Process* proc);
	virtual Process* get_next_process();
};

class RR_Scheduler : public Scheduler {
public:
	RR_Scheduler(int q);

};

