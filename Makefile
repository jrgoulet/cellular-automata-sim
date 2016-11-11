all:
	mpic++ -std=c++11 Simulator.cpp Node.cpp Row.cpp State.cpp main.cpp display.cpp -o forest -lncurses