CXXFLAGS:=--std=c++14
LIBS=-lpthread
main: main.o
	g++ $(CXXFLAGS) main.o -o main $(LIBS)

run_main: main
	for i in {1..1000}; do ./main; done | sort | uniq -c

producer_consumer: producer_consumer.o
	g++ $(CXXFLAGS) producer_consumer.o -o producer_consumer $(LIBS)

clean:
	rm -rf *.o main producer_consumer

.PHONY:
	clean
