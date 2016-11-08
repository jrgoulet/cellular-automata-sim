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
    int _color;

public:
    Node(int status);
    Node(int status, int color, int z);
    Node(int status, Row* r);
    void setn(int i, int s);
    void set(int s, int c);
    void set(int i);
    void setc(int i);
    std::array<int,8>* n();
    int status();
    int color();
    friend std::ostream& operator<<(std::ostream&, const Node&);
    friend std::string& operator+=(std::string&, const Node&);
};


#endif //FOREST_NODE_H
