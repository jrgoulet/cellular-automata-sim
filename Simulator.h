//
// Created by Joe Goulet on 11/1/16.
//
#include <vector>
#include "Node.h"
#include "defs.h"
#include <random>

#ifndef FOREST_SIMULATOR_H
#define FOREST_SIMULATOR_H



class Simulator {
    int _mode;
    char** _argv;
    ctrlv* _ctrlv;
    langv* _langv;
    std::string _name;
    //std::vector<std::vector<int>>* _memv;
    Simulator(Simulator const& copy);            // Not Implemented
    Simulator* operator=(Simulator const* copy);
    Simulator();
public:
    static Simulator* instance() {
        static Simulator instance;
        return &instance;
    }
    void run(Node* node);
    void init(char** argv);
    void set_default();
    //void set_mode(int mode);
    ctrlv* get_ctrlv();
    char translate(int q);
    void display();
    void set_conway();
    friend std::ostream& operator<<(std::ostream&, const Simulator&);
};


#endif //FOREST_SIMULATOR_H
