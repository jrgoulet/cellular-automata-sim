#include "defs.h"
#include <random>
#include "Simulator.h"
#include "State.h"
#include "Node.h"
#include <tuple>
#include <ncurses.h>

using namespace std;

/* Method declarations */
void forest_fire(Node* n);
void conway(Node* n);


/* RNG */
random_device rd;    /* seed for RNG */
mt19937 gen(rd());   /* mersenne twister (RNG) */


Simulator::Simulator() {
    _mode = 0;
    _ctrlv = new ctrlv();
    _langv = new langv();
}


void Simulator::init(char** argv) {
    _argv = argv;
}


void Simulator::set_default() {

    _name = "Forest Fire";
    _mode = 1;

    /* Simulation vars */
    var ignition    (stod(_argv[3]), "Ignition");
    var growth      (stod(_argv[4]), "Growth");
    _ctrlv->push_back(ignition);
    _ctrlv->push_back(growth);

    /* Language */
    for (char c : {' ','T','X'}) _langv->push_back(c);

    /* Colors */
    init_pair(0,COLOR_BLACK,COLOR_BLACK);
    init_pair(1,COLOR_GREEN,COLOR_BLACK);
    init_pair(2,COLOR_MAGENTA,COLOR_BLACK);
}

void Simulator::set_conway() {

    _name = "Conway's Game of Life";
    _mode = 2;

    /* Simulation vars */
    var u  (2, "Under-Population");
    var o  (3, "Over-Population");
    var g  (3, "Reproduction");
    _ctrlv->push_back(u);
    _ctrlv->push_back(o);
    _ctrlv->push_back(g);

    /* Language */
    for (char c : {' ','o'}) _langv->push_back(c);

    /* Colors */
    init_pair(0,COLOR_BLACK,COLOR_BLACK);
    init_pair(1,COLOR_GREEN,COLOR_BLACK);
}


/**
 * Coin toss using a random-device-seeded mersenne twister over a
 * distribution between 1 and 100,000. After playing with some RNGs,
 * I liked this one the most. This function returns a boolean with
 * a p probability of returning true.
 * @param p probability of occurrence
 * @return bool
 */
bool toss(double p) {
    dist d(1,100000);
    double r = (d((gen)) / 100000.0);
    //cout << r << " < " << p << " " << (r < p) << endl;
    return r < p;
}


/**
 * Returns a random integer in range
 * @param low range min
 * @param high range max
 * @return int
 */
int toss(int low, int high) {
    dist d(low,high);
    return (d((gen)));
}


void Simulator::run(Node* n) {
    if (_mode == 1) forest_fire(n);
    else if(_mode == 2) conway(n);
}


vector<tuple<double,string>>* Simulator::get_ctrlv() {
    return _ctrlv;
}


char Simulator::translate(int i) {
    return _langv->at(i);
}


int get_density(Node* n) {
    int d = 0;
    for(int x : *n->n()){ if(x == 1) d++;}
    return d;
}


void forest_fire(Node* n) {
    Simulator* s = Simulator::instance();
    double i = get<0>(s->get_ctrlv()->at(0));
    double g = get<0>(s->get_ctrlv()->at(1));

    bool ignited = false;
    if (n->status() == 1) {
        for (int x : *n->n()) {
            if (x == 2) {
                n->set(2, 2);
                ignited = true;
            }
        }
    }
    if (!ignited) {
        if (n->status() == 2) { n->set(0, 1); }
        else if (n->status() == 1) {
            if (toss(i)) { n->set(2, 2);}
        }
        else if (n->status() == 0) {
            if (toss(g*(get_density(n)+1))) { n->set(1, 1);}
        }
    }
}


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
