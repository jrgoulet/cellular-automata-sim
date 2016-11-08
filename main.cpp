#include "defs.h"
#include "State.h"
#include "Simulator.h"

using namespace std;

#define SCREEN_DELAY 40000000    /* For smooth viewing of simulation */

int main(int argc, char** argv) {

    /* thread state */
    State* s = new State(argc, argv);

    /* mode */
    Simulator* sim =Simulator::instance();
    sim->set_default();
    //sim->set_conway();

    /* run simulation */ /* State.cpp contains detailed flow */
    for (int i = 0; i < s->get_n(); i++) {
        s->transmit_nodes();
        s->update_neighbors();
        s->apply_simulation();
        s->display_map(SCREEN_DELAY);
        s->inc_n();
    }
    /* exit */
    quit();
}

