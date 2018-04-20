#include "Event.h"
#include "Process.h"
#include "Scheduler.h"
#include <list>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
//#include <unistd.h>       //linux/unix

using namespace std;

list<Event*> eventList;
list<Process*> procList;
list<Process*> blockList;
int CURRENT_TIME = 0;
bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS;
Scheduler* THE_SCHEDULER;
int* randvals;
int totalRands = 0;
int ofs = 0;
int verbose = false;
char type;

Event* get_event() {
	if (eventList.empty()) {
		return nullptr;
	}
	Event* event = eventList.front();
	eventList.pop_front();
	return event;
}

int get_next_event_time() {
	if (eventList.empty()) {
		return -1;
	}
	return eventList.front()->get_timestamp();
}

void put_event(Event* eve) {
	if (eventList.empty()) {
		eventList.push_back(eve);
		return;
	}
	int time = eve->get_timestamp();
	list<Event*>::iterator iter;
	iter = eventList.begin();
	while (iter != eventList.end()) {
		if (time < (*iter)->get_timestamp()) {
			break;
		}
		iter++;
	}
	eventList.insert(iter,eve);
}

void read_rand(string filename) {
	ifstream file;
	file.open(filename);
	int num;
	int randNum;
	file >> num;
	totalRands = num;
	randvals = new int[num];
	for (int i = 0; i < num; i++) {
		file >> randNum;
		randvals[i] = randNum;
	}
	file.close();
}

int myrandom(int burst) {
	return 1 + (randvals[(ofs++) % totalRands] % burst);
}

void read_inputfile(string filename) {
	ifstream file;
	file.open(filename);
	int at, tc, cb, ib;
	int pid = 0;
	while (file >> at) {
		file >> tc;
		file >> cb;
		file >> ib;
		int prio = myrandom(4);
		Process* proc = new Process(pid,at,tc,cb,ib,prio,at);
		procList.push_back(proc);
		pid++;
	}
}



void print_state(process_state st) {
	switch (st) {
	case STATE_CREATED:
		cout<< "CREATED";
		break;
	case STATE_READY:
		cout << "READY";
		break;
	case STATE_RUNNING:
		cout << "RUNNG";
		break;
	case STATE_BLOCKED:
		cout << "BLOCK";
		break;
	case STATE_PREEMPT:
		cout << "READY";
		break;
	case STATE_DONE:
		cout << "Done";
		break;
	}
}

void verbose_print(Event* evt) {
	
	if (verbose) {
		Process *proc = evt->get_process();
		CURRENT_TIME = evt->get_timestamp();
		int pid = proc->get_PID();
		cout << CURRENT_TIME << " " << pid << " " << proc->get_timeInPrevState() << ": ";

		switch (evt->get_transition()) {
		case TRANS_TO_READY:
			print_state(evt->get_oldState());
			cout << " -> READY"<<endl;
			break;
		case TRANS_TO_DONE:
			cout << "Done"<<endl;
			break;
		case TRANS_TO_BLOCK:
			print_state(evt->get_oldState());
			cout << " -> ";
			print_state(evt->get_newState());
			cout << " ib=" << proc->get_dynamicIB() << " rem=" << proc->get_rem() << endl;
			break;
		default:
			print_state(evt->get_oldState());
			cout << " -> ";
			print_state(evt->get_newState());
			cout << " cb=" << proc->get_cbRem() << " rem=" << proc->get_rem() << " prio=";
			if (type == 'P') {
				cout << proc->get_dynamicPrio() << endl;
			}
			else {
				cout << proc->get_staticPrio() - 1 << endl;
			}
			break;
		}
	}
}

void Simulation() {
	Event* evt;
	while ((evt = get_event())!=nullptr) {
		Process* proc = evt->get_process(); // this is the process the event works on
		CURRENT_TIME = evt->get_timestamp();

		proc->set_timeInPrevState(CURRENT_TIME - proc->get_state_ts());
		proc->set_state_ts(CURRENT_TIME);
		verbose_print(evt);

		switch (evt->get_transition()) { // which state to transition to?
		case TRANS_TO_DONE:
			proc->set_FT(CURRENT_TIME);
			THE_SCHEDULER->set_FT(CURRENT_TIME);
			CURRENT_RUNNING_PROCESS = nullptr;
			CALL_SCHEDULER = true;
			break;
		case TRANS_TO_READY:
			// must come from BLOCKED or from PREEMPTION
			// must add to run queue
			
			if (CURRENT_RUNNING_PROCESS == proc) {
				CURRENT_RUNNING_PROCESS = nullptr;
			}
			THE_SCHEDULER->add_process(proc);
			CALL_SCHEDULER = true; // conditional on whether something is run
			break;
		case TRANS_TO_RUN: {
			// create event for either preemption or blocking
			int rem = proc->get_rem();
			int quantum = THE_SCHEDULER->get_quantum();
			proc->set_CW(proc->get_CW() + proc->get_timeInPrevState());
			int CB = proc->get_cbRem();
			if (CB <= 0) {
				CB = myrandom(proc->get_CB());
				if (CB == 0) {
					CB += proc->get_CB();
				}
				proc->set_cbRem(CB);
			}


			if (CB > quantum) {
				//create event for preemption
				if (quantum >= rem) {
					proc->set_rem(0);
					Event* newevent = new Event(evt->get_newState(), STATE_DONE, proc, CURRENT_TIME + rem);
					put_event(newevent);

				}
				else {
					proc->set_rem(rem-quantum);
					proc->set_cbRem(CB - quantum);
					Event* newevent = new Event(evt->get_newState(), STATE_PREEMPT, proc, CURRENT_TIME + quantum);
					put_event(newevent);
				}
			}
			else {
				//create event for blocking
				if (CB >= rem) {
					proc->set_rem(0);
					Event* newevent = new Event(evt->get_newState(), STATE_DONE, proc, CURRENT_TIME + rem);
					put_event(newevent);
				}
				else {
					proc->set_rem(rem-CB);
					proc->set_cbRem(0);
					proc->set_dynamicIB(myrandom(proc->get_IB()));
					Event* newevent = new Event(evt->get_newState(), STATE_BLOCKED, proc, CURRENT_TIME + CB);
					put_event(newevent);
				}
			}
			break;
		}
			
		case TRANS_TO_BLOCK: {
			//create an event for when process becomes READY again
			int IB = proc->get_dynamicIB();
			proc->set_IT(proc->get_IT()+IB);
			proc->set_dynamicPrio(proc->get_staticPrio()-1);
			Event* newevent = new Event(evt->get_newState(), STATE_READY, proc, CURRENT_TIME + IB);
			put_event(newevent);

			Process* blockProc = new Process(CURRENT_TIME, IB);
			blockList.push_back(blockProc);
			CURRENT_RUNNING_PROCESS = nullptr;
			CALL_SCHEDULER = true;
			break;
		}
			
		case TRANS_TO_PREEMPT: {
			// add to runqueue(no event is generated)
			int prio = proc->get_dynamicPrio() - 1;
			if (prio < 0) {
				prio = proc->get_staticPrio() - 1;
				proc->set_reset(true);
			}
			proc->set_dynamicPrio(prio);
			CURRENT_RUNNING_PROCESS = nullptr;
			THE_SCHEDULER->add_process(proc);
			CALL_SCHEDULER = true;
			break;
		}
			
		}
		bool isPreempt = (evt->get_newState() == STATE_PREEMPT);
		//remove current event object from Memory
		delete evt;
		evt = nullptr;
		if (CALL_SCHEDULER) {
			if (get_next_event_time() == CURRENT_TIME) {
				continue;//process next event from Event queue
			}
			CALL_SCHEDULER = false;
			if (CURRENT_RUNNING_PROCESS == nullptr) {
				CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
				if (CURRENT_RUNNING_PROCESS == nullptr) {
					continue;
				}
				if (!isPreempt && CURRENT_RUNNING_PROCESS->get_cbRem()<=0) {
					int cpuburst = myrandom(CURRENT_RUNNING_PROCESS->get_CB());
					if (cpuburst == 0) {
						cpuburst += CURRENT_RUNNING_PROCESS->get_CB();
					}
					CURRENT_RUNNING_PROCESS->set_cbRem(cpuburst);
				}
				
				Event* newevent = new Event(STATE_READY, STATE_RUNNING, CURRENT_RUNNING_PROCESS, CURRENT_TIME);
				put_event(newevent);
			}
			// create event to make process runnable for same time.
		}
		
		
	}
}

void print_info(char type, int quantum) {
	// print the scheduler info
	switch (type) {
	case 'F':
		cout << "FCFS" << endl;
		break;
	case 'L':
		cout << "LCFS" << endl;
		break;
	case 'S':
		cout << "SJF" << endl;
		break;
	case 'P':
		cout << "PRIO " << quantum << endl;
		break;
	case 'R':
		cout << "RR " << quantum << endl;
		break;
	}

	// for each process print using this format print out 
	// the specifics of the process
	int procRunTime = 0;
	//int procNum = 0;
	int total_tt = 0;
	int total_cw = 0;
	for(Process* proc : procList)
	{
		int     id = proc->get_PID();            // unique id for this process
		int arrival = proc->get_AT();      // specified arrival time of process
		int totaltime = proc->get_TC();  // specified total exec time of process
		procRunTime += totaltime;
		int cpuburst = proc->get_CB();     // specified cpuburst of process
		int ioburst = proc->get_IB();      // specified ioburst of process
		int     static_prio = proc->get_staticPrio();   // static priority of the process 
								   // 0 if not prio scheduler
		int state_ts = proc->get_state_ts();   // time we entered into this state
		int iowaittime = proc->get_IT(); // total iowaittime of process during sim
		int cpuwaittime = proc->get_CW(); // time we were ready to run but did not

		total_tt += state_ts - arrival;
		total_cw += cpuwaittime;
		printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
			id,
			arrival, totaltime, cpuburst, ioburst, static_prio,
			state_ts, // last time stamp
			state_ts - arrival,
			iowaittime,
			cpuwaittime);
		//procNum++;
	}
	int begin = 0;
	int end = 0;
	int total_time = 0;
	for (Process* blockProc : blockList) {
		int proc_begin = blockProc->get_AT();
		int proc_end = proc_begin + blockProc->get_TC();
		if (proc_begin < end) {
			if (proc_end > end) {
				total_time += proc_end - end;
				end = proc_end;
			}
		}
		else {
			begin = proc_begin;
			end = proc_end;
			total_time += end - begin;
		}
	}

	// compute the following variables based on the simulation
	// and the final state of all the processes 

	int    maxfintime = THE_SCHEDULER->get_FT();
	double cpu_util = 100.0 * procRunTime/maxfintime;
	double io_util = 100.0 * total_time/maxfintime;
	double avg_turnaround = (double)total_tt/procList.size();
	double avg_waittime = (double)total_cw /procList.size();
	double throughput = 100.0 * procList.size()/maxfintime;

	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
		maxfintime,
		cpu_util,
		io_util,
		avg_turnaround,
		avg_waittime,
		throughput);

}

int main(int argc, char* argv[]) {
	verbose = false;
	//Linux/Unix getopt
	/*
	char *svalue = NULL;
	int index;
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, "vs:")) != -1)
		switch (c)
		{
		case 'v':
			verbose = true;
			break;
		case 's':
			svalue = optarg;
			break;

		case '?':
			if (optopt == 's'){
			    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			    return 1;
			}
			else if (isprint(optopt)){
			    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				return 1;
			}
			break;
		default:
			abort();
		}

		read_inputfile(argv[optind]);
		read_rand(argv[optind+1]);
		
		svalue
	*/

	//windows
	
	int quantum = 10000;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == 'v') {
			verbose = true;
		}
		else if (argv[i][0] == '-' && argv[i][1] == 's') {
			string temp;
			switch(argv[i][2]) {
			case 'F': {
				type = 'F';
				FCFS_Scheduler* fsche = new FCFS_Scheduler(10000);
				THE_SCHEDULER = fsche;
				break;
			}
			case 'L': {
			    type = 'L';
			    LCFS_Scheduler* lsche = new LCFS_Scheduler(10000);
			    THE_SCHEDULER = lsche;
			    break;
			}
			case 'S': {
				type = 'S';
				SJF_Scheduler* ssche = new SJF_Scheduler(10000);
				THE_SCHEDULER = ssche;
				break;
			}
			case 'P': {
				type = 'P';
				temp = argv[i];
				quantum = atoi(temp.substr(3).c_str());
				Prio_Scheduler* psche = new Prio_Scheduler(quantum);
				THE_SCHEDULER = psche;
				break;
			}
			case 'R': {
				type = 'R';
				temp = argv[i];
				quantum = atoi(temp.substr(3).c_str());
				RR_Scheduler* rsche = new RR_Scheduler(quantum);
				THE_SCHEDULER = rsche;
				break;
			}
				
			}
		}
		else {
		    read_rand(argv[i+1]);
			read_inputfile(argv[i]);
			break;
		}
	}
	
	for (Process* proc : procList) {
		Event* eve = new Event(STATE_CREATED, STATE_READY, proc, proc->get_AT());
		put_event(eve);
	}
	Simulation();
	print_info(type,quantum);
	

	//test
/*
	verbose = true;
	read_rand("rfile");
	read_inputfile("input6");
	Prio_Scheduler* sche = new Prio_Scheduler(5);
	type = 'P';
	THE_SCHEDULER = sche;
	for (Process* proc : procList) {
		Event* eve = new Event(STATE_CREATED, STATE_READY, proc, proc->get_AT());
		put_event(eve);
	}
	Simulation();
	print_info(type, 5);
	system("pause");
	*/
	return 0;
}
