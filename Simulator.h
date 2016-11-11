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
    // std::vector<std::vector<int>>* _memv;    // For when I decide to implement memory
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
    void display();
    char translate(int i);
    void set_conway();
    friend std::ostream& operator<<(std::ostream&, const Simulator&);
    friend std::string& operator += (std::string&, const Simulator&);
};


#endif //FOREST_SIMULATOR_H
