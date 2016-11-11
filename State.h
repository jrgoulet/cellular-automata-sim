#ifndef FOREST_STATE_H
#define FOREST_STATE_H

#include <vector>
#include "Row.h"
#include "Node.h"

class State {
    int _rank;           /* process rank */
    int _size;           /* number of processes */

    int _mode;           /* simulation mode */
    std::string _filename; /* map filename */
    int _height;         /* map height */
    int _width;          /* map width */
    double _density;     /* forest density (map generation) */

    int _win_height;      /* curses window height */
    int _win_width;

    int _generations;    /* number of generations */
    int _current;        /* current generation */
    double _ignition;    /* ignition probability */
    double _growth;      /* growth probability */

    int _start;          /* start row index */
    int _end;            /* end row index */
    int _top;            /* top thread neighbor */
    int _bot;            /* bottom thread neighbor */
    Row* _inner_top;     /* top row (local) */
    Row* _inner_bot;     /* bottom row (local) */
    Row* _outer_top;     /* top row (remote) */
    Row* _outer_bot;     /* bottom row (remote) */

    std::vector<Row*>* _trees; /* all local trees (nodes) */
    std::vector<std::string>* _map; /* initial map from file */

public:
    State(int argc, char **argv);

    /* initialization */
    void get_map();
    void init_window();
    void adjust_window_width(int w);
    void generate_map();
    void build_nodes();
    void set_bounds();
    void transmit_nodes();
    void update_neighbors();
    void apply_simulation();
    void check(int argc, char** argv);
    void fail(std::string e);
    void init_map();
    void inc_n();

    /* getters */
    Row* get_row(int i);
    int get_node_status(int row, int n);
    int get_node_color(int r, int n);
    int get_current_generation();

    /* display.cpp */
    void display_map(int delay);
    void display_exit();
    friend std::ostream& operator<<(std::ostream&, const State&);
    friend std::string& operator += (std::string&, const State&);
};
#endif //FOREST_STATE_H
