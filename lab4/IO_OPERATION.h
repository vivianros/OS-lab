#pragma once
class IO_OPERATION
{
public:
	int id;
	int arrival_time;
	int track;
	int start_time;
	int end_time;
	int length;
	IO_OPERATION(int i, int a,int t);
	~IO_OPERATION();
};

