#include "Scheduler.h"



Scheduler::Scheduler(int q)
{
	quantum = q;
}


Scheduler::~Scheduler()
{
}



int Scheduler::get_quantum() {
	return quantum;
};
int Scheduler::get_FT() {
	return FT;
};
void Scheduler::set_FT(int time) {
	FT = time;
};
void Scheduler::set_quantum(int q) {
	quantum = q;
};
void Scheduler::add_process(Process* proc) {
	readyList.push_back(proc);
};
Process* Scheduler::get_next_process() {
	if (readyList.empty()) {
		return nullptr;
	}
	Process* proc = readyList.front();
	readyList.pop_front();
	return proc;
};


FCFS_Scheduler::FCFS_Scheduler(int q):Scheduler(q)
{
	quantum = q;
}

LCFS_Scheduler::LCFS_Scheduler(int q) : Scheduler(q)
{
	quantum = q;
}

Process* LCFS_Scheduler::get_next_process() {
	if (readyList.empty()) {
		return nullptr;
	}
	Process* proc = readyList.back();
	readyList.pop_back();
	return proc;
};


SJF_Scheduler::SJF_Scheduler(int q) :Scheduler(q)
{
	quantum = q;
}

Process* SJF_Scheduler::get_next_process() {
	if (readyList.empty()) {
		return nullptr;
	}
	list<Process*>::iterator iter;
	iter = readyList.begin();
	Process* shortestJob = *iter;
	int shortest = (*iter)->get_rem();
	int pid = (*iter)->get_PID();
	iter++;
	while (iter != readyList.end()) {
		if ((*iter)->get_rem() < shortest) {
			shortest = (*iter)->get_rem();
			shortestJob = *iter;
			pid = (*iter)->get_PID();
		}
		else if ((*iter)->get_rem() == shortest) { //if two processes have same TC, FCFS
			if ((*iter)->get_PID() < pid) {
				pid = (*iter)->get_PID();
				shortestJob = *iter;
			}
		}
		iter++;
	}
	readyList.remove(shortestJob);
	return shortestJob;
};





Prio_Scheduler::Prio_Scheduler(int q) :Scheduler(q)
{
	quantum = q;
}

void Prio_Scheduler::add_process(Process* proc) {
	int index = proc->get_dynamicPrio();
	if (proc->get_reset()) {
		expiredList[index].push_back(proc);
	}
	else {
		activeList[index].push_back(proc);
	}
	
	proc->set_reset(false);
};


Process* Prio_Scheduler::get_next_process() {
	if (activeList[0].empty()&& activeList[1].empty()&& activeList[2].empty()&& activeList[3].empty()) {
		swap(activeList, expiredList);
	}
	if (activeList[0].empty() && activeList[1].empty() && activeList[2].empty() && activeList[3].empty()) {
		return nullptr;
	}
	int pIndex = 3;
	while (activeList[pIndex].empty()) {
		pIndex--;
	}
	Process* greatestJob = activeList[pIndex].front();
	activeList[pIndex].pop_front();

	return greatestJob;
};



RR_Scheduler::RR_Scheduler(int q) :Scheduler(q)
{
	quantum = q;
}

