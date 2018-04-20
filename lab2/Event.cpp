#include "Event.h"



Event::Event(process_state os, process_state ns, Process* p, int time)
{
	oldState = os;
	newState = ns;
	pro = p;
	timestamp = time;
}


Event::~Event()
{
}
Process* Event::get_process() {
	return pro;
}

int Event::get_timestamp() {
	return timestamp;
}

transitions Event::get_transition() {
	switch (newState) {
	case STATE_READY:
		return TRANS_TO_READY;
	case STATE_RUNNING:
		return TRANS_TO_RUN;
	case STATE_BLOCKED:
		return TRANS_TO_BLOCK;
	case STATE_PREEMPT:
		return TRANS_TO_PREEMPT;
	case STATE_DONE:
		return TRANS_TO_DONE;
	}
};

process_state Event::get_newState() {
	return newState;
};

process_state Event::get_oldState() {
	return oldState;
};