#ifndef FOREST_ROW_H
#define FOREST_ROW_H

#include <vector>
#include "Node.h"

class Node;

class Row {
    std::vector<int>* _intv;
    std::vector<Node*>* _nodev;

public:
    Row(int* i, int size);
    Row(std::vector<std::string>* map, int r);
    std::vector<int>* get_intv();
    std::vector<Node*>* get_nodev();
    Node* get_node(int i);
    void push(Node* n);
    int get(int i);
    void sync();
};
#endif //FOREST_ROW_H