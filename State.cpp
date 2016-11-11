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


/**
 * Default constructor
 * @param argc argc
 * @param argv argv
 * @return State object
 */
State::State(int argc, char **argv) {
    check(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_size);

    _filename = argv[1];
    _mode = (argc == 5) ? 1 : 2;
    _generations = stoi(argv[2]);
    _current = 1;
    _ignition = stod(argv[3]);
    _growth = stod(argv[4]);
    _width = (_mode == 2) ? stoi(argv[5]) : 0;
    _height = (_mode == 2) ? stoi(argv[6]): 0;
    _density = (_mode == 2) ? stod(argv[7]) : 0;
    
    init_map();
}


/**
 * Initializes the simulation's map.
 * Mode 1: Premade map
 * Mode 2: Generated map
 */
void State::init_map() {
    MPI_Status status;
    int buffer;
    if (_mode == 2) {
        if (_rank == 0) {
            generate_map();
            for (int i = 1; i < _size; i++) MPI_Send(&buffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        } else MPI_Recv(&buffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    get_map();
}


/**
 * Generates a map from passed arguments
 */
void State::generate_map() {
    fstream file;
    string row;

    file.open (_filename, fstream::out);
    if (!file) fail(ERROR_FILE);
    for (int i = 0; i < _height; i++) {
        row = "";
        for(int j = 0; j < _width; j++) {
            if (toss(_density)) row += "T";
            else row += " ";
        }
        file << row << endl;
    }
    file.close();
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

    init_window();
    set_bounds();
    build_nodes();
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
    _trees = new vector<Row *>();
    for (int i = _start; i < _end; i++) _trees->push_back(new Row(_map,i));
    _inner_top = _trees->front();
    _inner_bot = _trees->back();
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
    for (int i = 0; i < _trees->size(); i++) {
        Row* r = _trees->at(i);
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
            
            if (_bot == -1 && i ==_trees->size()-1) { for (int x : {5,6,7}) n->setn(x, 3); }
            else if (i ==_trees->size()-1) {
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
    for (Row* r : *_trees) { for (int j = 0; j < _width; j++) { Simulator::instance()->run(r->get_node(j)); }}
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
    return _trees->at(i);
}


/**
 * Status of a node in row r at index n
 * @param row row
 * @param n node index
 * @return int node status
 */
int State::get_node_status(int r, int n) {
    return _trees->at(r)->get(n);
}

int State::get_node_color(int r, int n) {
    return _trees->at(r)->get_node(n)->color();
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
        cout << "Mode 2: ./forest [filename] [# generations] [ignition probability] [growth probability] ";
        cout << "[width] [height] [density]" << endl;
    }
    quit();
}


/**
 * Checks arguments
 */
void State::check(int argc, char** argv) {
    if (argc != 5 && argc != 8) fail(ERROR_ARGV_C);
    try {
        if (stoi(argv[2]) < 1) fail(ERROR_ARGV_2);
        if (stod(argv[3]) < 0 || stoi(argv[3]) > 1) fail(ERROR_ARGV_3);
        if (stod(argv[4]) < 0 || stoi(argv[4]) > 1) fail(ERROR_ARGV_4);
        if (_mode == 2) {
            if (stoi(argv[5]) < 5) fail(ERROR_ARGV_5);
            if (stoi(argv[6]) < 5) fail(ERROR_ARGV_6);
            if (stod(argv[7]) < 0 || stod(argv[7]) > 1) fail(ERROR_ARGV_7);
        }
    } catch (int e) {
        fail(ERROR_ARGV_T);
    }
    Simulator::instance()->init(argv);
}


/**
 * Finalize MPI
 */
void quit() {
    MPI_Finalize();
    exit(0);
}