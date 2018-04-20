#pragma once

class Process
{
private:
	int rem; //remain process time
	int CB; //CPU Burst
	int IB; //IO Burst
	int dynamicIB;
	int PID;
	int AT;//Arrive Time
	int TC;//Total CPU Time
	int staticPrio; //priority
	int dynamicPrio;
	int FT; //finish time
	int IT; //IO Time (time in block state)
	int CW; //CPU waiting time( time in ready state)
	int timeInPrevState;
	int cbRem;
	int state_ts;
public:
	Process(int at, int tc);  // for io utilization
	Process(int pid, int at, int tc, int cb, int ib, int prio,int st_ts);
	~Process();
	int get_AT();
	int get_CB();
	int get_IB();
	int get_dynamicIB();
	int get_PID();
	int get_TC();
	int get_staticPrio();
	int get_dynamicPrio();
	int get_FT();
	int get_rem();
	int get_IT();
	int get_CW();
	int get_timeInPrevState();
	int get_cbRem();
	int get_state_ts();
	bool reset;//for prio, whether the dynamic priority has been reset
	bool get_reset();

	void set_timeInPrevState(int time);
	void set_cbRem(int rem);
	void set_state_ts(int ts);
	void set_FT(int time);
	void set_CW(int time);
	void set_rem(int r);
	void set_dynamicPrio(int p);
	void set_dynamicIB(int ib);
	void set_IT(int time);
	void set_reset(bool t);

};

