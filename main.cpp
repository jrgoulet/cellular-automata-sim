
#include "defs.h"
#include "State.h"
#include "Simulator.h"
#include <fstream>

using namespace std;

#define SCREEN_DELAY 40000000    /* For smooth viewing of simulation */


int main(int argc, char** argv) {

    /* thread state */
    State* s = new State(argc, argv);

    /* run simulation */ /* State.cpp contains detailed flow */
    for (int i = 0; i < s->get_current_generation(); i++) {
        s->transmit_nodes();
        s->update_neighbors();
        s->apply_simulation();
        s->display_map(SCREEN_DELAY);
        s->inc_n();
    }

    /* end simulation */
    s->display_exit();

    /* exit */
    quit();
}

