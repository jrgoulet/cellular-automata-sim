#include <mpi.h>
#include <vector>
#include <fstream>
#include <random>
#include <cstdlib>
#include <unistd.h>
#include "State.h"
#include "Simulator.h"
#include "defs.h"
#include <ncurses.h>

using namespace std;
extern string print_row(int thread, int row, int width, vector<Node*>* nodev);

/**
 * Default constructor
 * @param argc argc
 * @param argv argv
 * @return State object
 */
State::State(int argc, char **argv) {
    //check(argc, argv);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_size);

    _filename = argv[1];
    _mode = (argc == 5) ? 1 : 2;
    if (_mode == 2) {
        _current = 1;
        init_sim(_filename);
    }
    else {
        _generations = stoi(argv[2]);
        _current = 1;
        _ignition = stod(argv[3]);
        _growth = stod(argv[4]);
        get_map();
    }
}


/**
 * Initialize simulator from file
 */
void State::init_sim(string filename) {
    int mode;
    fstream file;
    file.open (filename, fstream::in);
    string line;

    /* Mode */
    getline(file, line);
    getline(file, line);
    mode = stoi(line);

    /* Height */
    getline(file, line);
    getline(file, line);
    _height = stoi(line);

    /* Width */
    getline(file, line);
    getline(file, line);
    _width = stoi(line);

    /* Generations */
    getline(file, line);
    getline(file, line);
    _generations = stoi(line);

    /* Forest Fire Simulation (Mode 1) */
    if (mode == 1) {
        double i, g, density;

        /* Ignition */
        getline(file, line);
        getline(file, line);
        i = stod(line);

        /* Growth */
        getline(file, line);
        getline(file, line);
        g = stod(line);

        /* Initial Density */
        getline(file, line);
        getline(file, line);
        density = stod(line);

        init_window();
        Simulator::instance()->set_forest(i,g);
        generate_nodes(0,1,density);
    }

    /* Conway Simulation (Mode 2) */
    if (mode == 2) {
        int u, o, g;
        double density;

        /* Underpopulation */
        getline(file, line);
        getline(file, line);
        u = stoi(line);

        /* Overpopulation */
        getline(file, line);
        getline(file, line);
        o = stoi(line);

        /* Growth */
        getline(file, line);
        getline(file, line);
        g = stoi(line);

        /* Initial Density */
        getline(file, line);
        getline(file, line);
        density = stod(line);

        init_window();
        Simulator::instance()->set_conway(u,o,g);
        generate_nodes(0,1,density);
    }

    set_bounds();
    build_nodes();

}


/**
 * Generates a node state map from passed arguments
 */
void State::generate_nodes(int min, int max, double density) {
    _node_map = new vector<Row*>();
    if (_rank == 0) {
        int send[_height][_width];
        for (int i = 0; i < _height; i++) {
            int nodes[_width];
            for (int j = 0; j < _width; j++) {
                int status = toss(density);
                nodes[j] = status;
                send[i][j] = status;
            }
            _node_map->push_back(new Row(nodes,_width));
        }
        for (int i = 1; i < _size; i++) {
            MPI_Send(&send,_width*_height,MPI_INT,i,0,MPI_COMM_WORLD);
        }
    }
    else {
        MPI_Status status;
        int recv[_height][_width];
        MPI_Recv(&recv,_width*_height,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        for (int i = 0; i < _height; i++) {
            int nodes[_width];
            for (int j = 0; j < _width; j++) {
                nodes[j] = recv[i][j];
            }
            _node_map->push_back(new Row(nodes,_width));
        }
    }
}


/**
 * Reads and converts a map from file
 */
void State::get_map() {
    _map = new vector<string>();
    fstream file;
    string row;

    file.open (_filename, fstream::in);
    if (!file) fail(ERROR_FILE);
    while (file) {
        getline(file, row);
        _map->push_back(row);
    }
    if (_map->at(_map->size()-1).length() == 0)
        _map->erase(_map->begin()+_map->size()-1);

    _height = (int) _map->size();
    _width = (int) _map->front().length();

    set_bounds();
    build_nodes();
    init_window();
    Simulator::instance()->set_forest(_ignition,_growth);

}


/**
 * Calculate thread boundaries (for any thread)
 * @param size total threads
 * @param rank  rank
 * @param height map height
 * @return tuple<int,int> of start, end row indexes
 */
tuple<int,int> get_bounds(int size, int rank, int height) {
    tuple<int,int> bounds;
    int* start = &get<0>(bounds);
    int* end = &get<1>(bounds);
    int workers = min(size, height);
    int work = height / workers;
    int remainder = height % workers;

    if (workers < rank + 1) {
        *start = -1;
        *end = -1;
    } else {
        *start = rank * work;
        *end = *start + work;
        int offset = min(rank, remainder);
        if (remainder > 0) {
            *start += offset;
            *end += offset;
            if (remainder >= rank + 1) (*end)++;
        }
    }
    return bounds;
}


/**
 * Store calculated thread boundaries.
 */
void State::set_bounds() {
    tuple<int,int> bounds = get_bounds(_size,_rank,_height);
    _start = std::get<0>(bounds);
    _end = std::get<1>(bounds);
    _top = _rank - 1;
    _bot = _rank + 1;
    if (_bot == _size || _bot == _height) _bot = -1;
}


/**
 * Builds node structure from map file for data inside boundaries.
 * Sets pointers to the top and bottom rows for transmission.
 */
void State::build_nodes() {
    _nodes = new vector<Row *>();
    if (_mode == 1) {
        for (int i = _start; i < _end; i++) _nodes->push_back(new Row(_map, i));
    }
    else {
        for (int i = _start; i < _end; i++) {
            _nodes->push_back(_node_map->at(i));
        }
    }

    _inner_top = _nodes->front();
    _inner_bot = _nodes->back();
}


/**
 * Send / receive border rows between threads. Blocks on receive, but not on send.
 */
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


/**
 * Updates the neighbor node array of each node in the thread's node vector.
 * Neighbor nodes are indexed 0-7 from top left to bottom right. If a position
 * is out of bounds, it is marked with a 3.
 */
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


/**
 * Applies the rules of the simulation for each generation on each node. I added a few
 * adjustable parameters to get more exciting simulations going.
 * @param burn The probability that a burning tree ignites an adjacent tree
 * @param grow A constant that allows an empty cell to produce a tree
 */
void State::apply_simulation() {
    for (Row* r : *_nodes) { for (int j = 0; j < _width; j++) { Simulator::instance()->run(r->get_node(j)); }}
}


/**
 * @return total generations
 */
int State::get_current_generation() {
    return _generations;
}


/**
 * @param i row number
 * @return Row* object
 */
Row* State::get_row(int i) {
    return _nodes->at(i);
}


/**
 * Status of a node in row r at index n
 * @param row row
 * @param n node index
 * @return int node status
 */
int State::get_node_status(int r, int n) {
    return _nodes->at(r)->get(n);
}

int State::get_node_color(int r, int n) {
    return _nodes->at(r)->get_node(n)->color();
}


/**
 * Increments the current generation (used in main)
 */
void State::inc_n() {
    _current++;
}


/**
 * Exit with error message
 * @param e
 */
void State::fail(string e) {
    if (_rank == 0) {
        cout << e << endl << "Usage:" << endl;
        cout << "Mode 1: ./forest [filename] [# generations] [ignition probability] [growth probability]" << endl;
        cout << "Mode 2: ./forest [.sim filename]" << endl;
    }
    quit();
}


/**
 * Checks arguments
 */
void State::check(int argc, char** argv) {
    if (argc != 5 && argc != 2) fail(ERROR_ARGV_C);
    try {
        if (stoi(argv[2]) < 1) fail(ERROR_ARGV_2);
        if (stod(argv[3]) < 0 || stoi(argv[3]) > 1) fail(ERROR_ARGV_3);
        if (stod(argv[4]) < 0 || stoi(argv[4]) > 1) fail(ERROR_ARGV_4);
    } catch (int e) {
        fail(ERROR_ARGV_T);
    }
}


/**
 * Finalize MPI
 */
void quit() {
    MPI_Finalize();
    exit(0);
}