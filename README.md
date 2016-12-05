# Cellular Automata Simulator #


### Description

A terminal-based simulator app that runs different types of simulations utilizing parallel processes handled by MPI

####Available modes:

 - Wildfire
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


To run a simulation from a `.sim` file:

    mpirun -np <num_threads> ./forest <.sim_file>


----------


### **.sim** files

#### Wildfire (Mode 1)

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

#### What is a Cellular Automaton?

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off (in contrast to a coupled map lattice). The grid can be in any finite number of dimensions. For each cell, a set of cells called its neighborhood is defined relative to the specified cell. An initial state (time t = 0) is selected by assigning a state for each cell. A new generation is created (advancing t by 1), according to some fixed rule (generally, a mathematical function) that determines the new state of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. Typically, the rule for updating the state of cells is the same for each cell and does not change over time, and is applied to the whole grid simultaneously, though exceptions are known, such as the stochastic cellular automaton and asynchronous cellular automaton.

Wikipedia

----------


#### Cellular Automata Simulations

Each simulation is governed by a set of rules that determines the state of each node in each generation. In basic simulations, state of a node is dependent upon the state of its neighbors. A famous example is Conway's Game of Life. Here are the rule sets for some basic simulations I've implemented in my simulator with screenshots.

#### Conway's Game of Life

    (i) Any live cell with fewer than two live neighbors dies, as if caused by under-population.
    (ii) Any live cell with two or three live neighbors lives on to the next generation.
    (iii) Any live cell with more than three live neighbors dies, as if by over-population.
    (iv) Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.

![Conway](http://i.imgur.com/gtjLpqu.png)

#### Forest Fire

    (i) A tree that is next to a burning tree will turn into a burning tree itself.
    (ii) A burning tree will turn into an empty space.
    (iii) A tree with no burning neighbors will ignite with probability II as a result of a lightning strike, where II is the ignition probability given to your program.
    (iv) An empty space will turn into a tree with probability G×(n+1)G×(n+1) where GG is the growth probability given to your program and nn is the number of non-burning trees neighboring this space.

![enter image description here](http://i.imgur.com/1VowliR.png)


----------


### The Simulation

The main idea is pretty simple. After initializing the simulator,  we have a grid of nodes split evenly between a given number of threads. If we have 16 threads and 32 rows, each thread is responsible for 2 rows. 

In order for us to update each node between each generation, we need to transmit the information of the node rows located on thread boundaries to their respective thread neighbors. This occurs in `transmit_nodes`. 

Next, in `update_neighbors`, each thread will update each of its nodes' neighbor array with the status of each of that node's neighbors.

Now, we have all of the information we need to apply the simulation for the current generation. In `apply_simulation`, we apply the rules of the simulation to each node.

In my simulator, I implement `NCurses` to display the grid in `display_map`. This was actually my first time using it, and I have to say, it's a pretty easy library to learn! I save the screen to a variable so that a user can save the output of the last generation printed to the terminal.

Finally, in `inc_n`, we increment the generation. It doesn't do much right now, but I plan on implementing some actions that will occur between each generation.

    int main(int argc, char** argv) {
    
        /* thread state */
        State* s = new State(argc, argv);
    
        /* run simulation */
        string out;
        for (int i = 0; i < s->get_current_generation(); i++) {
            s->transmit_nodes();
            s->update_neighbors();
            s->apply_simulation();
            out = s->display_map(SCREEN_DELAY);
            s->inc_n();
        }
    
        /* end simulation */
        s->display_exit(out);
    
        /* exit */
        quit();
    }

#### Transmitting the Nodes

Essentially, to talk to another process, you use `MPI_Send` to send a message to a thread. To receive an expected message, you call `MPI_Receive`. For non-blocking send and receive, simply append an '`I`' after the underscore.

In this function, we're sending the border rows of each process to its top and bottom neighbors (if any).

	void State::transmit_nodes() {
	    int send_top[_width], send_bot[_width], recv_top[_width], recv_bot[_width];
	
	    MPI_Request request;
	    if (_top > -1) for (int j = 0; j < _width; j++) send_top[j] = _inner_top->get(j);
	    if (_bot > -1) for (int j = 0; j < _width; j++) send_bot[j] = _inner_bot->get(j);
	    if (_top > -1) MPI_Isend(&send_top,_width,MPI_INT,_top,0,MPI_COMM_WORLD,&request);
	    if (_bot > -1) MPI_Isend(&send_bot,_width,MPI_INT,_bot,0,MPI_COMM_WORLD,&request);
	
	    MPI_Status status;
	    if (_top > -1) MPI_Recv(&recv_top,_width,MPI_INT,_top,0,MPI_COMM_WORLD,&status);
	    if (_bot > -1) MPI_Recv(&recv_bot,_width,MPI_INT,_bot,0,MPI_COMM_WORLD,&status);
	
	    if (_top > -1) _outer_top = new Row(recv_top,_width);
	    if (_bot > -1) _outer_bot = new Row(recv_bot,_width);
	}

#### Updating Each Node's Neighbors

Is there an easier way? Ah, I wish. In here, we have to treat each neighbor differently simply because each node corresponds to an `(X,Y)` coordinate on the graph. And of course, we need to account for edge cases (literally). 

	void State::update_neighbors() {
	    for (int i = 0; i < _nodes->size(); i++) {
	        Row* r = _nodes->at(i);
	        for (int j = 0; j < _width; j++) {
	            Node* n = r->get_node(j);
	            if (_top == -1 && i == 0) { for (int x : {0,1,2}) n->setn(x, 3); }
	            else if (i == 0) {
	                n->setn(0, (j == 0) ? 3 : _outer_top->get(j - 1));
	                n->setn(1, _outer_top->get(j));
	                n->setn(2, (j == _width - 1) ? 3 : _outer_top->get(j + 1));
	            } else {
	                n->setn(0, (j == 0) ? 3 : get_node_status(i - 1, j - 1));
	                n->setn(1, get_node_status(i - 1, j));
	                n->setn(2, (j == _width - 1) ? 3 : get_node_status(i - 1, j + 1));
	            }
	
	            n->setn(3, (j == 0) ? 3 : r->get(j - 1));
	            n->setn(4, (j == _width - 1) ? 3 : r->get(j + 1));
	            
	            if (_bot == -1 && i ==_nodes->size()-1) { for (int x : {5,6,7}) n->setn(x, 3); }
	            else if (i ==_nodes->size()-1) {
	                n->setn(5, (j == 0) ? 3 : _outer_bot->get(j - 1));
	                n->setn(6, _outer_bot->get(j));
	                n->setn(7, (j == _width - 1) ? 3 : _outer_bot->get(j + 1));
	            } else {
	                n->setn(5, (j == 0) ? 3 : get_node_status(i + 1, j - 1));
	                n->setn(6, get_node_status(i + 1, j));
	                n->setn(7, (j == _width - 1) ? 3 : get_node_status(i + 1, j + 1));
	            }
	        }
	        r->sync();
	    }
	}

----------

#### Running the Simulation

Ah, here's an easy one:

    void State::apply_simulation() {
        for (Row* r : *_nodes) { for (int j = 0; j < _width; j++) { Simulator::instance()->run(r->get_node(j)); }}
    }

Just kidding! Actually, this is where all the fun begins. Up to this point, you've seen the State object in action, which holds the state of the application before, during, and after the simulation. 

#### Simulator

Enter the Simulator! Here's what it looks like when we first initialize the application. We have a mode that corresponds to a specific simulation (i.e. Conway's Game of Life), a control vector containing the variables that govern the simulation, and a vector containing the language of the simulation. This language contains the symbols that correspond to each state displayed on the grid.

    Simulator::Simulator() {
        _mode = 0;
        _ctrlv = new ctrlv();
        _langv = new langv();
    }

For example, here is what initializing the Forest Fire simulation looks like:

	void Simulator::set_forest(double i, double g) {
	
	    _name = "Forest Fire";
	    _mode = 1;
	
	    /* Simulation vars */
	    var ignition    (i, "Ignition");
	    var growth      (g, "Growth");
	    _ctrlv->push_back(ignition);
	    _ctrlv->push_back(growth);
	
	    /* Language */
	    for (char c : {' ','T','X'}) _langv->push_back(c);
	
	    /* Colors */
	    init_pair(0,COLOR_BLACK,COLOR_BLACK);
	    init_pair(1,COLOR_GREEN,COLOR_BLACK);
	    init_pair(2,COLOR_RED,COLOR_BLACK);
	}

We initialize our simulation variables and define our language in here. 

Simulation variables are type-defined as `var`.  It represents `std::tuple<double,std::string> var`
The control vector `ctrlv` type is simply a vector of these vars: `std::vector<var> ctrlv`

You'll also notice init_pair. This is an `NCurses` function that defines the foreground and background of a color pair index. We use this color pair to change the color of an element displayed on the screen.


#### Mersenne Twister Pseudo RNG

Now, these simulations would be nothing without a good random number generator. I currently implement a Mersenne Twister to handle randomization. 

    /* RNG */
    random_device rd;    /* seed for RNG */
    mt19937 gen(rd());   /* mersenne twister (RNG) */

We use this generator to handle our tosses- whether we're looking for a boolean or an integer.

Boolean

    bool toss(double p) {
        dist d(1,100000);
        double r = (d((gen)) / 100000.0);
        //cout << r << " < " << p << " " << (r < p) << endl;
        return r < p;
    }

Integer

    int toss(int low, int high) {
        dist d(low,high);
        return (d((gen)));
    }


----------

#### Apply Simulation

Ok, back to the simulation! If you'll remember, our state object brought us here from 

    void State::apply_simulation() 

The first thing we do is determine what simulation we're running, and direct flow to that simulation's logic. 

    void Simulator::run(Node* n) {
        if (_mode == 1) forest_fire(n);
        else if(_mode == 2) conway(n);
    }

Let's say we're running Conway's Game of Life. That brings us here:

    void conway(Node* n) {
        Simulator* s = Simulator::instance();
        double u = get<0>(s->get_ctrlv()->at(0));
        double o = get<0>(s->get_ctrlv()->at(1));
        double g = get<0>(s->get_ctrlv()->at(2));
    
        int pop = get_density(n);
    
        if (n->status() == 1) {
            if (pop < u) n->set(0,0);
            if (pop > o) n->set(0,0);
        } else {
            if (pop == g) n->set(1,1);
        }
    }


----------

### Display

Now that we've handled the simulation, we can pop back a few stack frames and continue. The next step is displaying this information on the screen. We return to main and enter:

    out = s->display_map(SCREEN_DELAY);

We can play around with the screen delay for optimizing the number of generations we want to see actually occur on-screen. This value is in nanoseconds. Frame rate is machine-dependent, so if you experience considerable lag, play around with this value to slow the simulation down.

If you've tinkered around with parallel computing before, you may know that I/O can be a headache. Unfortunately, NCurses does not have native parallel support, so we're handling all of the display in process `0`.

In order to do this, all other processes must send their information to `Process 0` for output.

	/**
	 * Displays a live view or snapshot of the current generation, using MPI_Barrier to
	 * ensure that no threads are expecting stray messages. All threads send to master,
	 * where the data is displayed.
	 * @param delay
	 */
	string State::display_map(int delay) {
	    MPI_Barrier(MPI_COMM_WORLD);
	    string out = "";
	    if (_rank != 0) {   /* slave : send map */
	        for (int i = 0; i < _nodes->size(); i++) {
	            int send[_width*2];
	            for (int j = 0; j < _width; j++) send[j] = get_node_status(i, j);
	            for (int j = _width; j < _width*2; j++) send[j] = get_node_color(i, j - _width);
	            MPI_Send(&send,_width*2,MPI_INT,0,0,MPI_COMM_WORLD);
	        }
	    } else {    /* master : receive and display */
	        int row = 1;
	
	        /* Overwrite tiles with spaces */
	        erase();
	
	        /* Row 0 */
	        out += *this;
	        adjust_window_width((int) out.length());
	        mvwaddstr(stdscr,0,0,out.c_str());
	        out += "\n";
	
	        /* Master thread row display */
	        for (int j = 0; j < _nodes->size(); j++) {
	            display_row(0,row,_width, get_row(j)->get_nodev());
	            out += print_row(0,row,_width,get_row(j)->get_nodev()) + "\n";
	            row++;
	        }
	
	        /* Slave thread row display */
	        for (int j = 1; j < _height - _nodes->size(); j++) {
	            tuple<int,int> bounds = get_bounds(_size,j,_height);
	            int k = get<1>(bounds) - get<0>(bounds);
	            for (int l = 0; l < k; l++) {
	                int recv[_width*2];
	                MPI_Status status;
	                MPI_Recv(&recv,_width*2,MPI_INT,j,0,MPI_COMM_WORLD,&status);
	                Row* r = new Row(recv,_width);
	                display_row(j,row,_width,r->get_nodev());
	                out += print_row(j,row,_width,r->get_nodev()) + "\n";
	                row++;
	            }
	        }
	
	        /* Update tiles */
	        refresh();
	
	    }
	
	    /* delay for visibility, 4000000 ns = 25 FPS avg */
	    timespec t0, t1;
	    t0.tv_sec = 0;
	    t0.tv_nsec = delay;
	    nanosleep(&t0,&t1);
	
	    return out;
	}



### Planned Features

I have some ideas for some simulations I'd like to run, but I think the first thing that is going to happen is converting this into a full-blown web application with a nice user interface.

Sometime soon:

 - Q-State Life Simulation
 - General GFX Simulations
 - CUDA / GPU Processing
 - Three.js Integration

If you have any ideas you'd like to see, shoot me a line!