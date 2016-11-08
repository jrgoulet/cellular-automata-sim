#include "Node.h"

using namespace std;

Node::Node(int status) {
    _n = new array<int,8>;
    _state = status;
    _age = 0;
}

Node::Node(int status, Row* r) {
    _n = new array<int,8>;
    _age = 0;
    _state = status;
    r->push(this);
}

int Node::status() {
    return _state;
}

void Node::set(int s) {
    _state = s;
}

void Node::set(int i, int s) {
    _n->at(i) = s;
}

array<int,8>* Node::n() {
    return _n;
}

ostream& operator << (ostream& o, const Node& n) {
    o << "n : " << n._state << " : [ ";
    for (int i : *n._n) o << i << " ";
    o << "]";
    return o;
}