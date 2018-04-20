SHELL:=/bin/bash

sched: hw2.o event.o process.o scheduler.o
	module load gcc-6.2.0;\
	g++ -g hw2.o event.o process.o scheduler.o -o sched
hw2.o: hw2.cpp Event.h Process.h Scheduler.h
	module load gcc-6.2.0;\
	g++ -g -c hw2.cpp -o hw2.o
event.o: Event.cpp Event.h
	g++ -g -c Event.cpp -o event.o
process.o: Process.cpp Process.h
	g++ -g -c Process.cpp -o process.o
scheduler.o: Scheduler.cpp Scheduler.h
	module load gcc-6.2.0;\
	g++ -g -c Scheduler.cpp -o scheduler.o
clean: 
	rm sched hw2.o event.o process.o scheduler.o
