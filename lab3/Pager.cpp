#include "Pager.h"

Pager::Pager()
{
}


Pager::~Pager()
{
}

void Pager::print_aging_info() {
	
}

Frame* Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	Frame* frame = new Frame();
	return frame;
}

int Pager::myrands(int burst) {
	return (randvals[(ofs++) % totalRands] % burst);
}

FIFO_Pager::FIFO_Pager() {
}

Frame* FIFO_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	Frame* frame = frame_list.front();
	frame_list.pop_front();
	frame_list.push_back(frame);
	return frame;
}

Second_Chance_Pager::Second_Chance_Pager() {
}

Frame* Second_Chance_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	Frame* frame = new Frame();
	bool referenced = true;
	while (referenced) {
		frame = frame_list.front();
		frame_list.pop_front();
		frame_list.push_back(frame);
		Process* proc = &proc_list[frame->proc_id];
		PTE* pte = &proc->page_table[frame->vpage];
		if (pte->REFERENCED) {
			pte->REFERENCED = 0;
		}
		else {
			referenced = false;
		}
	}
	
	return frame;
}

Random_Pager::Random_Pager() {}

Frame* Random_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	int burst = frame_list.size();
	int pos = myrands(burst);
	Frame* frame = &frame_table[pos];
	return frame;
}

NRU_Pager::NRU_Pager() {
	count = 0;
}

Frame* NRU_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	//(referenced,modified)
	vector<Frame*> list[4];//0:(0,0)
						//1:(0,1)
						//2:(1,0)
						//3:(1,1)
	Frame* frame = new Frame();
	for (int i = 0; i < frame_list.size(); i++) {
		frame = &frame_table[i];
		Process* proc = &proc_list[frame->proc_id];
		PTE* pte = &proc->page_table[frame->vpage];
		if (pte->REFERENCED) {
			if (pte->MODIFIED) {      //(1,1)
				list[3].push_back(frame);
			}
			else {                    //(1,0)
				list[2].push_back(frame);
			}
		}
		else {
			if (pte->MODIFIED) {      //(0,1)
				list[1].push_back(frame);
			}
			else {                    //(0,0)
				list[0].push_back(frame);
			}
		}
	}
	int pos = 0;
	while (list[pos].size() == 0)
	{
		pos++;
	}

	int burst = list[pos].size();
	int result = myrands(burst);

	frame = list[pos][result];
	count++;

	//reset REFERENCED every 10th
	if (count == 10) {
		count = 0;
		for (int i = 0; i < frame_list.size(); i++) {
			Frame *reset_frame = &frame_table[i];
			Process* proc = &proc_list[reset_frame->proc_id];
			PTE* pte = &proc->page_table[reset_frame->vpage];
			pte->REFERENCED = 0;
		}
	}
	

	return frame;
}

Clock_Pager::Clock_Pager() {
	hand = 0;
}

Frame* Clock_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	Frame* frame = new Frame();
	int n = frame_list.size();
	bool referenced = true;
	while (referenced) {
		frame = &frame_table[hand];
		hand = (hand + 1) % n;
		Process* proc = &proc_list[frame->proc_id];
		PTE* pte = &proc->page_table[frame->vpage];
		if (pte->REFERENCED) {
			pte->REFERENCED = 0;
		}
		else {
			referenced = false;
		}
	}

	return frame;
}

Aging_Pager::Aging_Pager() {

}

Frame* Aging_Pager::determine_victim_frame(list<Frame*>& frame_list, Process *proc_list, Frame *frame_table) {
	Frame* frame = new Frame();
	unsigned int mask = 1<<31;
	unsigned int lowest = -1;
	int pos = 0;
	//initialize Rbits
	if (Rbits.size() == 0) {
		for (int i = 0; i < frame_list.size(); i++) {
			Rbits.push_back(0);
		}
	}
	for (int i = 0; i < frame_list.size(); i++) {
		Rbits[i] = Rbits[i] >> 1;
		frame = &frame_table[i];
		Process* proc = &proc_list[frame->proc_id];
		PTE* pte = &proc->page_table[frame->vpage];
		if (pte->REFERENCED) {
			Rbits[i] = Rbits[i] + mask;
			pte->REFERENCED = 0;
		}
		if (Rbits[i] < lowest) {
			lowest = Rbits[i];
			pos = i;
		}

	}
	selected = pos;
	frame = &frame_table[pos];
	
	return frame;
}

void Aging_Pager::print_aging_info() {
	cout << "ASTATUS: ";
	for (int i = 0; i < Rbits.size(); i++) {
		cout <<i<<": "<< (Rbits[i]>>28)<< (Rbits[i]<<4 >> 28) << (Rbits[i] << 8 >> 28) << (Rbits[i] << 12 >> 28) << (Rbits[i] << 16 >> 28) << (Rbits[i] << 20 >> 28) << (Rbits[i] << 24 >> 28) << (Rbits[i] << 28 >> 28) << " ";
	}
	cout <<"selected --> min_frame = "<<selected<<" age="<<Rbits[selected]<< endl;
}