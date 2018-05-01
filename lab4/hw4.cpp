#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<list>
#include<unistd.h>

#include "IO_SCHEDULER.h"
#include "IO_OPERATION.h"

using namespace std;

ifstream infile;
list<IO_OPERATION*> operation_list;
list<IO_OPERATION*> print_list;
//list<IO_OPERATION*> waiting_list;
IO_OPERATION *current_operation = nullptr;
IO_SCHEDULER *THE_SCHEDULER;
int current_time = 0;
int current_track = 0;
int tot_movement = 0;
bool v, q, f;

void print_result() {
	int count = 0;
	int total_time = current_time;
	double total_turnaround = 0.0;
	double total_waittime = 0.0;
	int max_waittime = 0;

	list<IO_OPERATION*>::iterator iter = print_list.begin();
	while (iter != print_list.end())
	{
		printf("%5d: %5d %5d %5d\n", count, (*iter)->arrival_time, (*iter)->start_time, (*iter)->end_time);
		total_turnaround += (*iter)->end_time - (*iter)->arrival_time;
		int wait = (*iter)->start_time - (*iter)->arrival_time;
		total_waittime += wait;
		if (wait > max_waittime) {
			max_waittime = wait;
		}
		*iter++;
		count++;
	}

	double avg_turnaround = total_turnaround / count;
	double avg_waittime = total_waittime / count;
	printf("SUM: %d %d %.2lf %.2lf %d\n",
		total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime);
}

void simulate() {
	if (v) {
		cout << "TRACE" << endl;
	}
	while (!(operation_list.empty() && THE_SCHEDULER->isEmpty() && current_operation == nullptr)) {
		current_time++;
		//add io operation
		if (operation_list.size()>0 && operation_list.front()->arrival_time == current_time) {
			if (v) {
				cout << current_time << ":\t" << operation_list.front()->id << " add " << operation_list.front()->track << endl;
			}
			THE_SCHEDULER->add_iooperation(operation_list.front());
			print_list.push_back(operation_list.front());
			operation_list.pop_front();
		}

		if (current_operation != nullptr) {
			                                                 //io active and complete
			if (current_operation->track == current_track) {
				if (v) {
					cout << current_time << ":\t" << current_operation->id << " finish " << current_operation->length << endl;
				}
				current_operation->end_time = current_time;
				current_operation = nullptr;
			}               			                      //io active but not complete
			else {
				//move the head
				tot_movement++;
				if (current_track < current_operation->track) {
					current_track++;
				}
				else {
					current_track--;
				}
			}
		}

		//fetch and start new io
		if (current_operation == nullptr) {
			current_operation = THE_SCHEDULER->get_next_operation(current_track);
			
			if (f) {
				THE_SCHEDULER->print_f(current_operation, current_time);
			}
			while (current_operation != nullptr) {
				current_operation->start_time = current_time;
				current_operation->length = abs(current_track - current_operation->track);
				if (v) {
					cout << current_time << ":\t" << current_operation->id << " issue " << current_operation->track << " " << current_track << endl;
				}
				//move the head
				if (current_track != current_operation->track) {
					tot_movement++;
					current_operation->length += 1;
					if (current_track < current_operation->track) {
						current_track++;
					}
					else {
						current_track--;
					}
					break;
				}
				else {
					current_operation->end_time = current_time;
					current_operation = THE_SCHEDULER->get_next_operation(current_track);
					if (v) {
						cout << current_time << ":\t" << current_operation->id << " issue " << current_operation->track << " " << current_track << endl;
					}
				}
			}
		}
	}
	print_result();
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

int main(int argc, char* argv[]) {
	string filename;
	char algo;
	v = false;
	q = false;
	f = false;
	
	int c;
	while ((c = getopt(argc, argv, "s:vqf")) != -1)
		switch (c)
		{
		case 's':
			algo = optarg[0];
			break;
		case 'v':
			v = true;
			break;
		case 'q':
			q = true;
			break;
		case 'f':
			f = true;
			break;
		}
	filename = argv[argc - 1];
	
	switch (algo)
	{
	case 'i':
		THE_SCHEDULER = new FIFO_SCHEDULER();
		break;
	case 'j':
		THE_SCHEDULER = new SSTF_SCHEDULER();
		break;
	case 's':
		THE_SCHEDULER = new LOOK_SCHEDULER();
		break;
	case 'c':
		THE_SCHEDULER = new CLOOK_SCHEDULER();
		break;
	case 'f':
		THE_SCHEDULER = new FLOOK_SCHEDULER();
		break;
	}

	infile.open(filename);
	string operation = get_next_line();
	int id = 0;
	while (operation.size() > 0) {
		int arrive;
		int track;
		stringstream(operation) >> arrive >> track;
		IO_OPERATION *io_operation = new IO_OPERATION(id,arrive,track);
		operation_list.push_back(io_operation);
		operation = get_next_line();
		id++;
	}
	infile.close();

	simulate();
	return 0;
}