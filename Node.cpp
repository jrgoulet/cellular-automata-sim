#include "Node.h"
#include "defs.h"
#include "Simulator.h"

using namespace std;

Node::Node(int status) {
    _n = new array<int,8>;
    _state = status;
    _age = 0;
    _color = 0;
}

Node::Node(int status, int color, int z) {
    _n = new array<int,8>;
    _state = status;
    _age = 0;
    _color = color;
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

void Node::setc(int i) {
    _color = i;
}

void Node::set(int s, int c) {
    _state = s;
    _color = c;
}

void Node::setn(int i, int s) {
    _n->at(i) = s;
}

string get_color(int i) {
    if (i==1) return FG_RED;
    else if (i==2) return FG_GREEN;
    else if (i==3) return FG_YELLOW;
    else if (i==4) return FG_BLUE;
    else if (i==5) return FG_MAGENTA;
    else if (i==6) return FG_CYAN;
    else if (i==7) return FG_WHITE;
    else if (i==8) return FG_BLACK;
    else return FG_DEFAULT;
}

array<int,8>* Node::n() {
    return _n;
}



ostream& operator << (ostream& o, const Node& n) {
    o << get_color(n._color) << Simulator::instance()->translate(n._state) << FG_DEFAULT;
    return o;
}

string& operator += (string& s, const Node& n) {
    return s += get_color(n._color) + Simulator::instance()->translate(n._state) + FG_DEFAULT;
}