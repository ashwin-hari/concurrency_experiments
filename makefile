CXXFLAGS:=--std=c++11
LIBS=-lpthread
main: main.o
	g++ $(CXXFLAGS) main.cpp -o main $(LIBS)

run_main: main
	for i in {1..1000}; do ./main; done | sort | uniq -c

clean:
	rm -rf *.o main

.PHONY:
	clean
