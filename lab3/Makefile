
mmu: hw3.o process.o pager.o frame.o
	g++ -g hw3.o process.o pager.o frame.o -o mmu
hw3.o: hw3.cpp Frame.h Process.h Pager.h
	g++ -g -c hw3.cpp -o hw3.o
pager.o: Pager.cpp Pager.h
	g++ -g -c Pager.cpp -o pager.o
process.o: Process.cpp Process.h
	g++ -g -c Process.cpp -o process.o
frame.o: Frame.cpp Frame.h
	g++ -g -c Frame.cpp -o frame.o
clean: 
	rm mmu hw3.o pager.o process.o frame.o
