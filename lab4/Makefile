
iosched: hw4.o IO_OPERATION.o IO_SCHEDULER.o
	g++ -g hw4.o IO_OPERATION.o IO_SCHEDULER.o -o iosched
hw4.o: hw4.cpp IO_OPERATION.h IO_SCHEDULER.h
	g++ -g -c hw4.cpp -o hw4.o
IO_OPERATION.o: IO_OPERATION.cpp IO_OPERATION.h
	g++ -g -c IO_OPERATION.cpp -o IO_OPERATION.o
IO_SCHEDULER.o: IO_SCHEDULER.cpp IO_SCHEDULER.h
	g++ -g -c IO_SCHEDULER.cpp -o IO_SCHEDULER.o
clean: 
	rm iosched hw4.o IO_OPERATION.o IO_SCHEDULER.o
