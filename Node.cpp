#include "Node.h"
#include "defs.h"
#include "Simulator.h"

using namespace std;

Node::Node(int status) {
    _n = new array<int,8>;
    _state = status;
    _age = 0;
    _color = status;
}

Node::Node(int status, Row* r) {
    _n = new array<int,8>;
    _age = 0;
    _state = status;
    r->push(this);
    _color = 0;
}

int Node::status() {
    return _state;
}

int Node::color() {
    return _color;
}

void Node::set(int s) {
    _state = s;
}

void Node::set(int s, int c) {
    _state = s;
    _color = c;
}

void Node::setn(int i, int s) {
    _n->at(i) = s;
}

array<int,8>* Node::n() {
    return _n;
}



ostream& operator << (ostream& o, const Node& n) {
    o << Simulator::instance()->translate(n._state) ;
    return o;
}

string& operator += (string& s, const Node& n) {
    return s += + Simulator::instance()->translate(n._state);
}