//
// Created by Joe Goulet on 10/30/16.
//

#ifndef FOREST_NODE_H
#define FOREST_NODE_H

#include <iostream>
#include <array>
#include "Row.h"

class Row;

class Node {
    int _state;
    std::array<int,8>* _n;  /* neighbors from top-left to bottom-right */
    int _age;

public:
    Node(int status);
    Node(int status, Row* r);
    void set(int i, int s);
    void set(int i);
    std::array<int,8>* n();
    int status();
    friend std::ostream& operator<<(std::ostream&, const Node&);
};


#endif //FOREST_NODE_H
