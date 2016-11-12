# **forest** Cellular Automata Simulator #


### Description
A fun, multi-threaded simulator app that runs different types of simulations.
####Available modes:

 - Forest Fire
 - Conway's Game of Life
 - Coming Soon: Q-State Life


----------


### Requirements

 - Unix
 - MPI
 - NCurses


----------


### Usage

 After downloading the repository:
 

    make

To run a forest fire simulation from a `.txt` file (deprecated, will be removed 11-18-2016):

    mpirun -np <num_threads> ./forest <filename> <num_generations> <ignition> <growth>

To run a simulation from a `.sim` file:

    mpirun -np <num_threads> ./forest <.sim_file>


----------


### **.sim** files

#### Forest Fire (Mode 1)

    mode:
    1
    height:
    50
    width:
    150
    generations:
    100
    ignition:
    0.001
    growth:
    0.01
    init density:
    0.05

#### Conway's Game of Life (Mode 2)

    mode:
    1
    height:
    50
    width:
    150
    generations:
    100
    underpopulation:
	2
	overpopulation:
	3
	growth:
	3
	init density:
	0.25

----------


### Screenshots
####Forest Fire
![Forest Fire](http://i.imgur.com/1VowliR.png)
####Conway's Game of Life
![Conway](http://i.imgur.com/gtjLpqu.png)