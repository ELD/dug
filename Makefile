# Author: Eric Dattore

dug:
	g++ -std=c++14 src/* -I/u/wy/iq/edattore/.local/include -L/u/wy/iq/edattore/.local/lib -lboost_system -lboost_program_options -o dug -static

clean:
	rm -f dug

