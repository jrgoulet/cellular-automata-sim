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

    int _generations;    /* number of generations */
    int _current;        /* current generation */
    double _ignition;    /* ignition probability */
    double _growth;      /* growth probability */
    double _burn_osc;    /* oscillating burn constant */
    double _grow_osc;    /* oscillating grow constant */
    int _wind_dir;


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
    void check(int argc, char** argv);
    int status(int row, int n);
    Row* get_row(int i);
    void get_map();
    void generate_map();
    void build_nodes();
    void set_bounds();
    void display_config();
    void transmit_nodes();
    void update_neighbors();
    void apply_simulation();
    void display_map(int delay);
    void fail(std::string e);
    void init_map();
    int get_n();
    void inc_n();
    friend std::ostream& operator<<(std::ostream&, const State&);
};
#endif //FOREST_STATE_H
