#include <random>
#include "Simulator.h"
#include "defs.h"
#include <tuple>

using namespace std;

random_device rd;    /* seed for RNG */
mt19937 gen(rd());   /* mersenne twister (RNG) */

Simulator::Simulator() {
    _mode = 0;
    _ctrlv = new ctrlv();
    _langv = new vector<char>();
}

void Simulator::init(char** argv) {
    _argv = argv;
}


void Simulator::set_default() {

    _mode = 1;
    _name = "Forest Fire";

    var ignition (stod(_argv[3]), "Ignition");
    var growth (stod(_argv[4]), "Growth");
    
    _ctrlv->push_back(ignition);
    _ctrlv->push_back(growth);

    for (char c : {' ','T','X'}) _langv->push_back(c);

    cout << *this << endl;

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
/**
 * Returns the display symbol for an associated state
 * @param q state
 * @return char
 */
char Simulator::translate(int q) {
    return _langv->at(q);
}

void Simulator::run(Node* n) {
    if (_mode == 1) { forest_fire(n); }
}

vector<tuple<double,string>>* Simulator::get_ctrlv() {
    return _ctrlv;
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
                n->set(2);
                ignited = true;
            }
        }
    }
    if (!ignited) {
        if (n->status() == 2) { n->set(0); }
        else if (n->status() == 1) {
            if (toss(i)) { n->set(2);}
        }
        else if (n->status() == 0) {
            if (toss(g*(get_density(n)+1))) { n->set(1);}
        }
    }
}

/**
 * Output for debugging purposes
 * Fun with iterators
 */
ostream& operator << (ostream& o, const Simulator& s) {
    for (int i = 0; i < 30; i++) o << "-"; o << endl;
    o << s._name << " (Mode " << s._mode << ")" << endl;
    o << "Vars" << "\t" << get<1>(s._ctrlv->at(0)) << ": " << get<0>(s._ctrlv->at(0)) << endl;
    for(ctrlv::iterator i = s._ctrlv->begin()+1; i != s._ctrlv->end(); i++) {
        o << "    \t" << get<1>(*i) << ": " << get<0>(*i) << endl;
    }
    o << "Map\t";
    for (int i = 0; i < s._langv->size(); i++) o << i << " ";
    o << endl << "   \t";
    for (char c : *s._langv) o << c << " ";
    o << endl;
    for (int i = 0; i < 30; i++) o << "-"; o << endl;
    return o;
}

