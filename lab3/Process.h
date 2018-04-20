#pragma once

struct PTE
{
	unsigned int VALID : 1;
	unsigned int WRITE_PROTECT : 1;
	unsigned int MODIFIED : 1;
	unsigned int REFERENCED : 1;
	unsigned int PAGEDOUT : 1;
	unsigned int ACCESSIBLE : 1;

	unsigned int FRAMEINDEX : 7;

	unsigned int FILE_MAPPED : 1;

	PTE() {
		VALID = 0;
		MODIFIED = 0;
		REFERENCED = 0; 
		PAGEDOUT = 0; 
		ACCESSIBLE = 0;
	}

	void set_data(unsigned int wp, unsigned int fm) {
		WRITE_PROTECT = wp;
		FILE_MAPPED = fm;
		ACCESSIBLE = 1;
	}
};

struct PSTATE {
	unsigned long int unmaps;
	unsigned long int maps;
	unsigned long int ins;
	unsigned long int outs;
	unsigned long int fins;
	unsigned long int fouts;
	unsigned long int zeros;
	unsigned long int segv;
	unsigned long int segprot;

	PSTATE() {
		unmaps = 0;
		maps = 0;
		ins = 0;
		outs = 0;
		fins = 0;
		fouts = 0;
		zeros = 0;
		segv = 0;
		segprot = 0;
	};
};

class Process
{
public:
	PSTATE pstats;
	PTE page_table[64];
	int proc_id;
	Process();
	~Process();
};

