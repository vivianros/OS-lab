#include "Process.h"


Process::Process(int at, int tc) {
	AT = at;
	TC = tc;
};

Process::Process(int pid, int at, int tc, int cb, int ib, int prio, int st_ts)
{
	PID = pid;
	AT = at;
	TC = tc;
	CB = cb;
	IB = ib;
	staticPrio = prio;
	dynamicPrio = prio - 1;
	rem = tc; // initial remain time = total cpu time
	state_ts = st_ts;
	timeInPrevState = 0;
	IT = 0;
	CW = 0;
	cbRem = 0;
	reset = false;
}


Process::~Process()
{
}

int Process::get_AT() {
	return AT;
};
int Process::get_CB() {
	return CB;
};
int Process::get_IB() {
	return IB;
};
int Process::get_dynamicIB() {
	return dynamicIB;
};
int Process::get_PID() {
	return PID;
};
int Process::get_TC() {
	return TC;
};
int Process::get_staticPrio() {
	return staticPrio;
};
int Process::get_dynamicPrio() {
	return dynamicPrio;
};
int Process::get_FT() {
	return FT;
};
int Process::get_rem() {
	return rem;
};
int Process::get_IT() {
	return IT;
};
int Process::get_CW() {
	return CW;
};
int Process::get_timeInPrevState() {
	return timeInPrevState;
};
int Process::get_cbRem() {
	return cbRem;
};
int Process::get_state_ts() {
	return state_ts;
};
bool Process::get_reset() {
	return reset;
}
void Process::set_timeInPrevState(int time) {
	timeInPrevState = time;
};
void Process::set_cbRem(int rem) {
	cbRem = rem;
};
void Process::set_state_ts(int ts) {
	state_ts = ts;
};
void Process::set_FT(int time) {
	FT = time;
};
void Process::set_CW(int time) {
	CW = time;
};
void Process::set_rem(int r) {
	rem = r;
};
void Process::set_dynamicPrio(int p) {
	dynamicPrio = p;
};

void Process::set_dynamicIB(int ib) {
	dynamicIB = ib;
};
void Process::set_IT(int time) {
	IT = time;
};
void Process::set_reset(bool t) {
	reset = t;
}
