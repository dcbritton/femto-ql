# A simple Makefile

main: src/main.cpp
	g++ -std=c++11 src/main.cpp -o main.o

run: main.o
	./main.o

clean:
	rm main.o
