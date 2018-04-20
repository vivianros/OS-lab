#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<list> 
#include<unistd.h>
#include "Frame.h"
#include "Pager.h"
#include "Process.h"

using namespace std;

const int cost_mapunmap = 400;
const int cost_pageinout = 3000;
const int cost_fileinout = 2500;
const int cost_zero = 150;
const int cost_segv = 240;
const int cost_segprot = 300;
const int cost_readwrite = 1;
const int cost_contextswitch = 121;

ifstream infile;
ifstream rfile;
Pager *THE_PAGER;
list<string> instruction_list;
int proc_num;
Process *proc_list;
Process *current_process;
list<Frame*> free_list;
int frame_num;
Frame *frame_table;
bool O, P, F, S, x, y, f, a;
unsigned long int instru_num = 0;
unsigned long int ctx_switches = 0;
unsigned long long int cost = 0;

Frame *allocate_frame_from_free_list() {
	if (free_list.size() == frame_num) {
		return NULL;
	}
	Frame *frame = &frame_table[free_list.size()];
	frame->frame_id = free_list.size();
	free_list.push_back(frame);
	return frame;
}

Frame *get_frame() {
	Frame *frame = allocate_frame_from_free_list();
	if (frame == NULL) {
		frame = THE_PAGER->determine_victim_frame(free_list, proc_list, frame_table);
		if (a) {
			THE_PAGER->print_aging_info();
		}
	} 
	return frame;
}

void context_switch(int proc_id) {
	current_process = &proc_list[proc_id];
	cost = cost + cost_contextswitch;
	ctx_switches = ctx_switches+1;
}

void print_pt() {
	for (int i = 0; i < proc_num; i++) {
		printf("PT[%d]: ", i);
		Process *p = &proc_list[i];
		for (int i = 0; i < 64; i++) {
			if (p->page_table[i].VALID) {
				cout << i << ":";
				string r = p->page_table[i].REFERENCED ? "R" : "-";
				string m = p->page_table[i].MODIFIED ? "M" : "-";
				string s = p->page_table[i].PAGEDOUT ? "S" : "-";
				cout << r << m << s << " ";
			}
			else {
				if (p->page_table[i].PAGEDOUT) {
					cout << "# ";
				}
				else {
					cout << "* ";
				}
			}
		}
		cout << endl;
	}

}

void print_ft() {
	cout << "FT: ";
	for (int i = 0; i < frame_num; i++) {
		Frame* frame = &frame_table[i];
		if (frame->proc_id == -1) {
			printf("* ");
		}
		else {
			printf("%d:%d ", frame->proc_id, frame->vpage);
		}
	}
	cout << endl;
}

void ref_page(char operation, int vpage) {
	PTE* current_pte = &current_process->page_table[vpage];
	if (!current_pte->ACCESSIBLE) {
		current_process->pstats.segv = current_process->pstats.segv+1;
		cost = cost + cost_segv + cost_readwrite;
		cout << " SEGV" << endl;
		return;
	}
	if (current_pte->VALID) {
		current_pte->REFERENCED = 1;
	}
	else {
		
		Frame* frame = get_frame();
		if (frame->proc_id != -1) {
			//UNMAP

			Process* prev_proc = &proc_list[frame->proc_id];
			PTE* prev_pte = &(prev_proc->page_table[frame->vpage]);
			cost = cost + cost_mapunmap;
			prev_proc->pstats.unmaps = prev_proc->pstats.unmaps+1;
			if (O) {
				printf(" UNMAP %d:%d\n", prev_proc->proc_id, frame->vpage);
			}

			//reset previous pte
			prev_pte->VALID = 0;
			prev_pte->REFERENCED = 0;
			if (prev_pte->MODIFIED) {
				if (prev_pte->FILE_MAPPED) {
					prev_proc->pstats.fouts = prev_proc->pstats.fouts+1;
					cost = cost + cost_fileinout;
					if (O) {
						cout << " FOUT" << endl;
					}
				}
				else {
					prev_pte->PAGEDOUT = 1;
					prev_proc->pstats.outs = prev_proc->pstats.outs+1;
					cost = cost + cost_pageinout;
					if (O) {
						cout << " OUT" << endl;
					}
				}
			}

		}
		
		frame->proc_id = current_process->proc_id;
		frame->vpage = vpage;
		//reset current pte
		current_pte->VALID = 1;
		current_pte->REFERENCED = 1;
		current_pte->MODIFIED = 0;
		current_pte->FRAMEINDEX = frame->frame_id;

		if (current_pte->FILE_MAPPED) {
			current_process->pstats.fins = current_process->pstats.fins+1;
			cost = cost + cost_fileinout;
			if (O) {
				cout << " FIN" << endl;
			}
		}
		else if (current_pte->PAGEDOUT) {
			current_process->pstats.ins = current_process->pstats.ins+1;
			cost = cost + cost_pageinout;
			if (O) {
				cout << " IN" << endl;
			}
		}
		else {
			current_process->pstats.zeros = current_process->pstats.zeros+1;
			cost = cost + cost_zero;
			if (O) {
				cout << " ZERO" << endl;
			}
		}
		current_process->pstats.maps= current_process->pstats.maps + 1;
		cost = cost + cost_mapunmap;
		if (O) {
			printf(" MAP %d\n", frame->frame_id);
		}

	}
	cost = cost + cost_readwrite;
	if (operation == 'w' && current_pte->WRITE_PROTECT) {
		current_process->pstats.segprot = current_process->pstats.segprot+1;
		cost = cost + cost_segprot;
		cout << " SEGPROT" << endl;
	}
	else if(operation == 'w') {
		current_pte->MODIFIED = 1;
	}
	//print current page table
	if (x) {
		printf("PT[%d]: ", current_process->proc_id);
		for (int i = 0; i < 64; i++) {
			if (current_process->page_table[i].VALID) {
				cout << i << ":";
				string r = current_process->page_table[i].REFERENCED ? "R" : "-";
				string m = current_process->page_table[i].MODIFIED ? "M" : "-";
				string s = current_process->page_table[i].PAGEDOUT ? "S" : "-";
				cout << r << m << s << " ";
			}
			else {
				if (current_process->page_table[i].PAGEDOUT) {
					cout << "# ";
				}
				else {
					cout << "* ";
				}
			}
		}
		cout << endl;
	}
	//print page table of all processes
	if (y) {
		print_pt();
	}
	//print frame table
	if (f) {
		print_ft();
	}
}

void get_next_instruction(char operation, int vpage) {
	if (O) {
		printf("%d: ==> %c %d\n",instru_num, operation, vpage);
	}
	switch (operation)
	{
	case 'c':
		context_switch(vpage);
		break;
	case 'r':
		ref_page('r',vpage);
		break;
	case 'w':
		ref_page('w',vpage);
		break;
	default:
		break;
	}
}

void simulate() {
	while (!instruction_list.empty()) {
		string instruction = instruction_list.front();
		instruction_list.pop_front();
		char operation;
		int vpage;
		stringstream(instruction) >> operation >> vpage;
		get_next_instruction(operation, vpage);

		instru_num++;
	}
		
}

string get_next_line() {
	string buffer;
	while (getline(infile, buffer)) {
		if (buffer.size() > 0 && buffer[0] != '#') {
			return buffer;
		}
	}
	return "";
}

void print_sum() {
	for (int i = 0; i < proc_num; i++) {
		Process* proc = &proc_list[i];
		PSTATE* pstats = &proc->pstats;
		printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
			proc->proc_id,
			pstats->unmaps, pstats->maps, pstats->ins, pstats->outs,
			pstats->fins, pstats->fouts, pstats->zeros, pstats->segv, pstats->segprot);
	}
	printf("TOTALCOST %lu %lu %llu\n", ctx_switches, instru_num, cost);
}

int main(int argc, char* argv[]) {
	O = P = F = S = x = y = f = a = false;
	char algo;
	string options;
	string fnum;
	int c;
	while ((c = getopt(argc, argv, "a:o:f:")) != -1)
		switch (c)
		{
		case 'a':
			algo = optarg[0];
			break;
		case 'o':
			options = optarg;
			break;
		case 'f':
			fnum = optarg;
			break;
		}
	frame_num = stoi(fnum);
	string filename = argv[argc - 2];
	string rfilename = argv[argc - 1];

	frame_table = new Frame[frame_num];

	infile.open(filename);

	switch (algo)
	{
	case 'f':
		THE_PAGER = new FIFO_Pager();
		break;
	case 's':
		THE_PAGER = new Second_Chance_Pager();
		break;
	case 'r':
		THE_PAGER = new Random_Pager();
		break;
	case 'n':
		THE_PAGER = new NRU_Pager();
		break;
	case 'c':
		THE_PAGER = new Clock_Pager();
		break;
	case 'a':
		THE_PAGER = new Aging_Pager();
		break;
	default:
		break;
	}

	//get options
	for (int i = 0; i < options.length(); i++) {
		switch (options[i])
		{
		case 'O':
			O = true;
			break;
		case 'P':
			P = true;
			break;
		case 'F':
			F = true;
			break;
		case 'S':
			S = true;
			break;
		case 'x':
			x = true;
			break;
		case 'y':
			y = true;
			break;
		case 'f':
			f = true;
			break;
		case 'a':
			a = true;
			break;
		default:
			break;
		}
	}

	string buffer = get_next_line();	
	proc_num = stoi(buffer);
	proc_list = new Process[proc_num];
	//read process
	for (int i = 0; i < proc_num; i++) {

		Process *proc = new Process();
		proc->proc_id = i;

		buffer = get_next_line();
		int vmaNum = stoi(buffer);
		for (int j = 0; j < vmaNum; j++) {
			buffer = get_next_line();
			//initialize process pagetable
			int start_page;
			int end_page;
			int write_protect;
			int filemapped;
			stringstream(buffer) >> start_page >> end_page >> write_protect >> filemapped;
			for (int k = start_page; k <= end_page; k++) {
				proc->page_table[k].set_data(write_protect, filemapped);
			}
		}

		proc_list[i] = *proc;
	}
	//read instructions
	buffer = get_next_line();
	while (buffer.size()>0) {
		char operation;
		int vpage;
		instruction_list.push_back(buffer);
		buffer = get_next_line();
	}
	infile.close();

	//read rfile
	rfile.open(rfilename);
	int num;
	int randNum;
	rfile >> num;
	int totalRands = num;
	int *randvals = new int[num];
	for (int i = 0; i < num; i++) {
		rfile >> randNum;
		randvals[i] = randNum;
	}
	THE_PAGER->ofs = 0;
	THE_PAGER->randvals = randvals;
	THE_PAGER->totalRands = totalRands;
	rfile.close();
	
	simulate();

	if (P) {
		print_pt();
	}
	if (F) {
		print_ft();
	}
	if (S) {
		print_sum();
	}
	return 0;
}