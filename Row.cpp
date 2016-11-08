#include<vector>
#include "Row.h"

using namespace std;

Node* Row::get_node(int i) {
    return _nodev->at(i);
}

Row::Row(int* i, int size) {
    _nodev = new vector<Node*>();
    _intv = new vector<int>();
    for (int j = 0; j < size; j++) {
        _nodev->push_back(new Node(i[j]));
        _intv->push_back(i[j]);
    }
}

Row::Row(vector<string>* map, int r) {
    _nodev = new vector<Node*>();
    _intv = new vector<int>();
    for (char c : map->at(r)) {
        int i;
        switch (c) {
            case 'T':
                i = 1;
                break;
            case 'X':
                i = 2;
                break;
            default:
                i = 0;
                break;
        }
        _nodev->push_back(new Node(i));
        _intv->push_back(i);
    }
}

void Row::push(Node* n) {
    _nodev->push_back(n);
}

int Row::get(int i) {
    return _nodev->at(i)->status();
}

void Row::sync() {
    _intv->clear();
    for (Node* n : *_nodev) _intv->push_back(n->status());
}

vector<int>* Row::get_intv() {
    return _intv;
}