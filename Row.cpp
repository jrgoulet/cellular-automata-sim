#include<vector>
#include "Row.h"

using namespace std;


/**
 * Constructor for int vector
 * @param i int vector
 * @param size size of vector
 * @return Row object
 */
Row::Row(int* i, int size) {
    _nodev = new vector<Node *>();
    _intv = new vector<int>();
    for (int j = 0; j < size; j++) {
        _nodev->push_back(new Node(i[j]));
        _intv->push_back(i[j]);
    }
}


/**
 * Constructor for map vector
 * @param map map vector
 * @param r map row
 * @return Row object
 */
Row::Row(vector<string>* map, int r) {
    _nodev = new vector<Node*>();
    _intv = new vector<int>();
    for (char c : map->at((unsigned long) r)) {
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


/**
 * Add node to node vector
 * @param n node
 */
void Row::push(Node* n) {
    _nodev->push_back(n);
}


/**
 * Get node from node vector
 * @param i node index
 * @return Node object
 */
Node* Row::get_node(int i) {
    return _nodev->at((unsigned long) i);
}


/**
 * Get node status from node vector
 * @param i node index
 * @return int node status
 */
int Row::get(int i) {
    return _nodev->at((unsigned long) i)->status();
}


/**
 * Synchronize node vector and int vector
 */
void Row::sync() {
    _intv->clear();
    for (Node* n : *_nodev) _intv->push_back(n->status());
}


/**
 * Get int vector
 * @return int vector
 */
vector<int>* Row::get_intv() {
    return _intv;
}


/**
 * Get node vector
 * @return node vector
 */
vector<Node*>* Row::get_nodev() {
    return _nodev;
}