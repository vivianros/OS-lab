#pragma once
#include "Process.h"

typedef enum { STATE_CREATED, STATE_READY, STATE_RUNNING, STATE_BLOCKED, STATE_PREEMPT, STATE_DONE }process_state;
typedef enum { TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT,TRANS_TO_DONE}transitions;


class Event
{
private:
	Process* pro;
	process_state newState;
	process_state oldState;
	int timestamp;
	transitions transition;
public:
	Event(process_state os, process_state ns, Process* p, int time);
	~Event();
	Process* get_process();
	int get_timestamp();
	transitions get_transition();
	process_state get_newState();
	process_state get_oldState();
};

