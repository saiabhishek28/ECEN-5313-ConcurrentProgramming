CFLAGS = -Wall -g -std=c++11
CXX = g++

all: sort

sort:
	$(CXX) main.cpp sorts.cpp -o mysort -pthread $(CFLAGS)

clean:
	rm mysort