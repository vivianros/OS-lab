#pragma once
#include<iostream>
#include<list>
#include<vector>
#include "Frame.h"
#include "Process.h"

using namespace std;

class Pager
{
public:
	int ofs;
	int *randvals;
	int totalRands;
	Pager();
	~Pager();
	int myrands(int burst);
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
	virtual void print_aging_info();
};

class FIFO_Pager:public Pager {
public:
	FIFO_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
};

class Second_Chance_Pager :public Pager {
public:
	Second_Chance_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
};

class Random_Pager :public Pager {
public:
	Random_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
};

class NRU_Pager :public Pager {
public:
	int count;
	NRU_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
};

class Clock_Pager :public Pager {
public:
	int hand;
	Clock_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
};

class Aging_Pager :public Pager {
public:
	vector<unsigned int> Rbits;
	int selected;
	Aging_Pager();
	virtual Frame* determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table);
	void print_aging_info();
};
