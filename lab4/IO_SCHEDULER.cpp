#include "IO_SCHEDULER.h"



IO_SCHEDULER::IO_SCHEDULER()
{
}


IO_SCHEDULER::~IO_SCHEDULER()
{
}

IO_OPERATION* IO_SCHEDULER::get_next_operation(int cur_track) {
	return nullptr;
}

void IO_SCHEDULER::add_iooperation(IO_OPERATION* operation) {
	waiting_list.push_back(operation);
}

bool IO_SCHEDULER::isEmpty() {
	return waiting_list.empty();
}

void IO_SCHEDULER::print_f(IO_OPERATION* operation, int cur_time) {
}

FIFO_SCHEDULER::FIFO_SCHEDULER() {
}

IO_OPERATION* FIFO_SCHEDULER::get_next_operation(int cur_track) {
	if (waiting_list.empty()) {
		return nullptr;
	}
	IO_OPERATION *oper = waiting_list.front();
	waiting_list.pop_front();
	return oper;
}

SSTF_SCHEDULER::SSTF_SCHEDULER() {
}

IO_OPERATION* SSTF_SCHEDULER::get_next_operation(int cur_track) {
	if (waiting_list.empty()) {
		return nullptr;
	}
	int min_distance = 10000;
	list<IO_OPERATION*>::iterator iter = waiting_list.begin();
	list<IO_OPERATION*>::iterator min_pointer;
	while (iter != waiting_list.end()) {
		int distance = abs((*iter)->track - cur_track);
		if (distance < min_distance) {
			min_pointer = iter;
			min_distance = distance;
		}
		*iter++;
	}
	IO_OPERATION *oper = *min_pointer;
	waiting_list.erase(min_pointer);
	return oper;
}

LOOK_SCHEDULER::LOOK_SCHEDULER() {
	direction = true;
}

IO_OPERATION* LOOK_SCHEDULER::get_next_operation(int cur_track) {
	if (waiting_list.empty()) {
		return nullptr;
	}
	list<IO_OPERATION*>::iterator pointer;
	if (direction) {                                         //direction from track 0 to max track
		int min_track = 10000;
		list<IO_OPERATION*>::iterator iter = waiting_list.begin();
		while (iter != waiting_list.end()) {
			if ((*iter)->track >= cur_track) {
				if ((*iter)->track < min_track) {
					min_track = (*iter)->track;
					pointer = iter;
				}
			}
			*iter++;
		}
		if (min_track == 10000) {						//no IO operation found, change the direction
			direction = !direction;
			return get_next_operation(cur_track);
		}
	}
	else {													//direction from max track to track 0
		int max_track = -1;
		list<IO_OPERATION*>::iterator iter = waiting_list.begin();
		while (iter != waiting_list.end()) {
			if ((*iter)->track <= cur_track) {
				if ((*iter)->track > max_track) {
					max_track = (*iter)->track;
					pointer = iter;
				}
			}
			*iter++;
		}
		if (max_track == -1) {						//no IO operation found, change the direction
			direction = !direction;
			return get_next_operation(cur_track);
		}
	}

	IO_OPERATION *oper = *pointer;
	waiting_list.erase(pointer);
	return oper;
}

CLOOK_SCHEDULER::CLOOK_SCHEDULER() {
}

IO_OPERATION* CLOOK_SCHEDULER::get_next_operation(int cur_track) {
	if (waiting_list.empty()) {
		return nullptr;
	}
	list<IO_OPERATION*>::iterator pointer = waiting_list.begin();

	int min_track = 10000;
	list<IO_OPERATION*>::iterator min_iter = waiting_list.begin();
	while (min_iter != waiting_list.end()) {                                //find the min track which is bigger than current track
		if ((*min_iter)->track >= cur_track) {
			if ((*min_iter)->track < min_track) {
				min_track = (*min_iter)->track;
				pointer = min_iter;
			}
		}
		*min_iter++;
	}
	if (min_track == 10000) {                                     //no track bigger than current track
																  //find the smallest track
		list<IO_OPERATION*>::iterator iter = waiting_list.begin();
		while (iter != waiting_list.end()) {
			if ((*iter)->track < min_track) {
				min_track = (*iter)->track;
				pointer = iter;
			}
			*iter++;
		}

	}
	IO_OPERATION *oper = *pointer;
	waiting_list.erase(pointer);
	return oper;
}

FLOOK_SCHEDULER::FLOOK_SCHEDULER() {
	active = 0;
	direction = true;
}

bool FLOOK_SCHEDULER::isEmpty() {
	return (io_queue[0].empty() && io_queue[1].empty());
}

void FLOOK_SCHEDULER::add_iooperation(IO_OPERATION* operation) {
	io_queue[(active+1)%2].push_back(operation);
}

IO_OPERATION* FLOOK_SCHEDULER::get_next_operation(int cur_track) {
	if (io_queue[0].empty() && io_queue[1].empty()) {
		return nullptr;
	}
	if (io_queue[active].empty()) {
		active = (active + 1) % 2;
	}

	list<IO_OPERATION*>::iterator pointer;
	if (direction) {                                         //direction from track 0 to max track
		int min_track = 10000;
		list<IO_OPERATION*>::iterator iter = io_queue[active].begin();
		while (iter != io_queue[active].end()) {
			if ((*iter)->track >= cur_track) {
				if ((*iter)->track < min_track) {
					min_track = (*iter)->track;
					pointer = iter;
				}
			}
			*iter++;
		}
		if (min_track == 10000) {						//no IO operation found, change the direction
			direction = !direction;
			return get_next_operation(cur_track);
		}
	}
	else {													//direction from max track to track 0
		int max_track = -1;
		list<IO_OPERATION*>::iterator iter = io_queue[active].begin();
		while (iter != io_queue[active].end()) {
			if ((*iter)->track <= cur_track) {
				if ((*iter)->track > max_track) {
					max_track = (*iter)->track;
					pointer = iter;
				}
			}
			*iter++;
		}
		if (max_track == -1) {						//no IO operation found, change the direction
			direction = !direction;
			return get_next_operation(cur_track);
		}
	}

	IO_OPERATION *oper = *pointer;
	io_queue[active].erase(pointer);
	return oper;
	
}

void FLOOK_SCHEDULER::print_f(IO_OPERATION* operation, int cur_time) {
	cout << cur_time << ":\t" << operation->id << " get Q=" << active << endl;
}